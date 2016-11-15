#define _GNU_SOURCE
static inline int strCmp( char const *a , char const *b ){ while( *a != '\0' && *a++ == *b++ ); return *a == *b && *b == '\0'; }
static inline void strcopy( char *dst , char const *src ){ while( *dst++ = *src++ ); }
#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/wait.h>
#include <linux/futex.h>
#include <sys/mman.h>
#include <sched.h>
void initWindow()
{
    initscr();
    raw();
    noecho();
    keypad( stdscr , TRUE );
}
void releaseWindow()
{
    endwin();
}
typedef struct
{
	int percent_complete;
	int complete;
	int src;
	int dst;
	size_t block_size;
	void *buffer;
	void *stack;
	size_t stack_size;
	int th_pid;
} CopyTask;
int threadRoutine( void *data )
{
	if( !data )
	{
		return;
	}
	CopyTask *task = ( CopyTask* )data;
	int len = 0;
	size_t total_read = 0;
	struct stat stat;
	fstat( task->src , &stat );
	size_t file_size = stat.st_size;
	while( len = read( task->src , task->buffer , task->block_size ) )
	{
		total_read += len;
		write( task->dst , task->buffer , len );
		task->percent_complete = ( total_read * 100 ) / file_size;
		usleep( 1000 );
	}
	task->complete = 1;
	close( task->dst );
	close( task->src );
	return 0;
}
void createTask( CopyTask *task , int src_fd , int dst_fd , size_t stack_size , size_t block_size )
{
	task->src = src_fd;
	task->dst = dst_fd;
	task->stack_size = stack_size;
	task->stack = mmap( NULL , stack_size , PROT_READ | PROT_WRITE , MAP_PRIVATE | MAP_ANONYMOUS  , -1 , 0 );
	task->buffer = malloc( block_size );
	task->block_size = block_size;
	task->th_pid = clone( threadRoutine , task->stack + stack_size - 1 , CLONE_VM | CLONE_FS , task );
}
void destroyTask( CopyTask task )
{
	waitpid( task.th_pid , NULL , 0 );
	free( task.buffer );
	munmap( task.stack , task.stack_size );
}
int main( int argc , char **argv )
{
	if( !argv[ 1 ] )
	{
		printf( "invalid input\n" );
	}
	int in_fd = open( argv[ 1 ] , O_RDONLY );
	if( in_fd <= 0 )
	{
		printf( "input file not found\n" );
		return -1;
	}
	char out_name[ 0x100 ];
	stpcpy( out_name , argv[ 1 ] );
	int len = strlen( argv[ 1 ] );
	char const *postfix = ".copy";
	stpcpy( out_name + len , postfix );
	int out_fd = open( out_name , O_WRONLY | O_TRUNC | O_CREAT );
	if( out_fd <= 0 )
	{
		close( in_fd );
		printf( "couldn't touch output file\n" );
		return -1;
	}
	CopyTask task = { 0 };
	createTask( &task , in_fd , out_fd , 0x1000 , 0x10 );
	initWindow();
    while( !task.complete )
    {
		int width , height;
		getmaxyx( stdscr , height , width );
		int i;
		for( i = 0; i < task.percent_complete; i++ )
		{
			mvprintw( 0 , i , "#" );
		}
        refresh();
    }
    exit:
	destroyTask( task );
    releaseWindow();
    return 0;
}