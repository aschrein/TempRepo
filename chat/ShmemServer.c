#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/signal.h>
#define BUFFER_WIDTH 0x100
#include "SharedMessages.h"
int working = 1;
void release()
{
	working = 0;
}
int main( int argc , char **argv )
{
	signal( SIGINT , release );
	int shared_fd = shm_open( "shared.data" , O_RDWR | O_CREAT | O_TRUNC , S_IRUSR | S_IWUSR );
	if( shared_fd < 0 )
	{
		perror( "error while opening shared file" );
		return -1;
	}
	ftruncate( shared_fd , SHARED_MEM_SIZE );
	MessageBoard *msg_board = mmap( NULL , SHARED_MEM_SIZE , PROT_READ | PROT_WRITE , MAP_SHARED , shared_fd , 0 );
	close( shared_fd );
	msg_board->nodes[ 0 ].len = -1;
	sem_init( &msg_board->semaphore , 0 , 1 );
	initscr();
    raw();
    noecho();
    keypad( stdscr , TRUE );
	nodelay( stdscr , 1 );
	while( working )
	{
		int ch = getch();
		if( ch == 3 )
		{
			break;
		} else
		{
			int width , height;
			getmaxyx( stdscr , height , width );
			int x = 0 , y = 0;
			for(; x < width; x++ )
			{
				for( y = 0; y < height; y++ )
				{
					mvprintw( y , x , " " );
				}
			}
			mvprintw( 0 , 0 , "chat server, ctrl-c to exit" );
			mvprintw( 1 , 0 , "clients:" );
			mvprintw( 1 , 10 , "messages:" );
			for( x = 0; x < width; x++ ) mvprintw( 2 , x , "_" );
			for( y = 3; y < height; y++ )
			{
				mvprintw( y , 10 , "|" );
			}
			sem_wait( &msg_board->semaphore );
			{
				y = 3;
				UserList *ulist = getUserList( msg_board );
				int i = 0;
				for(; i < ulist->count; i++ )
				{
					if( ulist->ids[ i ] >= 0 ) mvprintw( y++ , 0 , "%i" , i );
				}
			}
			{
				y = 3;
				MessageNode *cur = msg_board->nodes;
				int offset = 0;
				while( cur->len > 0 )
				{
					offset++;
					cur = &cur->raw[ 0 ] + cur->len;
				}
				offset = height - offset - 3;
				cur = msg_board->nodes;
				while( cur->len > 0 )
				{
					if( offset > 0 ) mvprintw( y++ , 11 , "%i:%s" , cur->user_id , cur->raw );
					offset++;
					cur = &cur->raw[ 0 ] + cur->len;
				}
			}
			sem_post( &msg_board->semaphore );
		}
		refresh();
		usleep( 1000 );
	}
	endwin();
	printf( "exited normalliy\n" );
	sem_destroy( &msg_board->semaphore );
	munmap( msg_board , SHARED_MEM_SIZE );
	shm_unlink( "shared.data" );
	return 0;
}