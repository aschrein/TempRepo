#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ncurses.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#define BUFFER_WIDTH 0x100
#ifdef SHARED
typedef struct
{
	int user_id;
	int len;
	char raw[];
} MessageNode;
typedef struct
{
	sem_t semaphore;
	int msg_count;
	MessageNode nodes[];
}
#else
typedef struct Message_t
{
	char *msg;
	struct Message_t *next , *prev;
} Message;
#endif
typedef struct Task_t
{
	struct Task_t *next , *prev;
	struct Server_t *server;
	pthread_t handle;
	int client_socket;
	int working;
} Task;
typedef struct Server_t
{
	Task *task_head;
	Message *msg_head;
	pthread_mutex_t mutex;
	pthread_cond_t garbage_signal;
	int working;
} Server;
void responserRoutine( Task *task )
{
	char buf[ BUFFER_WIDTH + 1 ];
	while( task->server->working )
	{
		int res = recv( task->client_socket , buf , BUFFER_WIDTH , 0 );
		if( res <= 0 )
		{
			break;
		}
		buf[ res ] = 0;
		//printf( "msg %s\n" , buf );
		addMessage( task->server , task->client_socket , buf );
	}
	pthread_mutex_lock( &task->server->mutex );
	task->working = 0;
	pthread_cond_signal( &task->server->garbage_signal );
	pthread_mutex_unlock( &task->server->mutex );
}
void garbageCollectorRoutine( Task *task )
{
	Server *server = task->server;
	while( task->server->working )
	{
		pthread_mutex_lock( &server->mutex );
		pthread_cond_wait( &server->garbage_signal , &server->mutex );
		Task *cur = server->task_head;
		while( cur )
		{
			Task *next = cur->next;
			if( !cur->working )
			{
				//printf( "task trashed\n" );
				if( cur->prev )
				{
					cur->prev->next = cur->next;
				} else
				{
					server->task_head = cur->next;
				}
				if( cur->next )
				{
					cur->next->prev = cur->prev;
				}
				if( cur->client_socket > 0 ) close( cur->client_socket );
				pthread_join( cur->handle , NULL );
				free( cur );
			}
			cur = next;
		}
		pthread_mutex_unlock( &server->mutex );
	}
	task->working = 0;
}
void rendererRoutine( Task *task )
{
	initscr();
    raw();
    noecho();
    keypad( stdscr , TRUE );
	Server *server = task->server;
	nodelay( stdscr , 1 );
	
	while( task->server->working )
	{
		int ch = getch();
		pthread_mutex_lock( &server->mutex );
		if( ch == 3 )
		{
			task->server->working = 0;
			pthread_cond_signal( &server->garbage_signal );
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
			{
				int y = 3;
				int offset = 0;
				Task *cur = server->task_head;
				while( cur )
				{
					offset++;
					cur = cur->next;
				}
				offset = height - offset - 3;
				cur = server->task_head;
				while( cur )
				{
					if( offset > 0 ) if( cur->client_socket > 0 ) mvprintw( y++ , 0 , "client:%i" , cur->client_socket );
					offset++;
					cur = cur->next;
				}
			}
			{
				int offset = 0;
				Message *cur = server->msg_head;
				while( cur )
				{
					offset++;
					cur = cur->next;
				}
				offset = height - offset - 3;
				cur = server->msg_head;
				int y = 3;
				while( cur )
				{
					if( offset > 0 ) mvprintw( y++ , 11 , cur->msg );
					offset++;
					cur = cur->next;
				}
			}
		}
		pthread_mutex_unlock( &server->mutex );
		refresh();
		usleep( 1000 );
	}
	endwin();
	task->working = 0;
}
#ifdef SHARED
void addMessage( Server *server , int user_id , char *raw_msg )
{
	pthread_mutex_lock( &server->mutex );
	Message *msg = ( Message* )malloc( sizeof( Message ) );
	memset( msg , 0 , sizeof( Message ) );
	msg->msg = ( char * )malloc( strlen( raw_msg ) + 0x10 );
	sprintf( msg->msg , "%i:%s" , user_id , raw_msg );
	if( !server->msg_head )
	{
		server->msg_head = msg;
	} else
	{
		Message *cur = server->msg_head;
		while( cur->next ) cur = cur->next;
		cur->next = msg;
		msg->prev = cur;
	}
	pthread_mutex_unlock( &server->mutex );
}
#else
void addMessage( Server *server , int user_id , char *raw_msg )
{
	pthread_mutex_lock( &server->mutex );
	Message *msg = ( Message* )malloc( sizeof( Message ) );
	memset( msg , 0 , sizeof( Message ) );
	msg->msg = ( char * )malloc( strlen( raw_msg ) + 0x10 );
	sprintf( msg->msg , "%i:%s" , user_id , raw_msg );
	if( !server->msg_head )
	{
		server->msg_head = msg;
	} else
	{
		Message *cur = server->msg_head;
		while( cur->next ) cur = cur->next;
		cur->next = msg;
		msg->prev = cur;
	}
	pthread_mutex_unlock( &server->mutex );
}	
#endif
/*void removeMessage( Server *server , Message *msg )
{
	pthread_mutex_lock( &server->mutex );
	if( msg->prev )
	{
		msg->prev->next = msg->next;
	} else
	{
		server->msg_head = msg->next;
	}
	if( msg->next )
	{
		msg->next->prev = msg->prev;
	}
	free( msg->msg );
	free( msg );
	pthread_cond_signal( &server->cond );
	pthread_mutex_unlock( &server->mutex );
}
/*void removeTask( Task *task )
{
	pthread_mutex_lock( &server->mutex );
	if( task->prev )
	{
		task->prev->next = task->next;
	} else
	{
		server->task_head = task->next;
	}
	if( task->next )
	{
		task->next->prev = task->prev;
	}
	if( task->client_sock > 0 ) close( task->client_sock );
	pthread_join( task->handle , NULL );
	free( task );
	pthread_cond_signal( &server->cond );
	pthread_mutex_unlock( &server->mutex );
}*/
void addTask( Server *server , void *routine , int socket )
{
	pthread_mutex_lock( &server->mutex );
	Task *task = ( Task* )malloc( sizeof( Task ) );
	memset( task , 0 , sizeof( Task ) );
	task->client_socket = socket;
	task->working = 1;
	task->server = server;
	pthread_create( &task->handle , NULL , routine , task );
	if( !server->task_head )
	{
		server->task_head = task;
	} else
	{
		Task *cur = server->task_head;
		while( cur->next ) cur = cur->next;
		cur->next = task;
		task->prev = cur;
	}
	pthread_mutex_unlock( &server->mutex );
}
int main( int argc , char **argv )
{
#ifdef SHARED
	int sock = socket( AF_INET , SOCK_STREAM | SOCK_NONBLOCK , 0 );
	int shared_fd = shm_open( "shared.data" , O_RDWR | O_CREAT | O_TRUNC , S_IRUSR | S_IWUSR );
	ftruncate( shared_fd , 0x100000 );
	void *shared_mem = mmap( NULL , 0x100000 , PROT_READ | PROT_WRITE , MAP_SHARED , shared_fd , 0 );
	close( shared_fd );
#endif
	if( sock < 0 )
	{
		perror( "could not create socket" );
		return -1;
	}
	{
		struct sockaddr_in sa =
		{
			sin_family : AF_INET ,
			sin_port : htons( 8080 ) ,
			sin_addr : { s_addr : inet_addr( "127.0.0.1" ) }
		};
		bind( sock , ( struct sockaddr* )&sa , sizeof( sa ) );
		listen( sock , 0x10 );
	}
	Server *server = ( Server* )malloc( sizeof( Server ) );
	{
		memset( server , 0 , sizeof( Server ) );
		pthread_mutex_init( &server->mutex , NULL );
		pthread_cond_init( &server->garbage_signal , NULL );
		server->working = 1;
		addTask( server , garbageCollectorRoutine , -1 );
		addTask( server , rendererRoutine , -1 );
	}
	while( server->working )
	{
		struct timeval timeval =
		{
			tv_sec : 0 ,
			tv_usec : 100 * 1000
		};
		fd_set s;
		FD_ZERO( &s );
		FD_SET( sock , &s );
		int num = select( sock + 1 , &s , NULL , NULL , &timeval );
		if( FD_ISSET( sock , &s ) )
		{
			int client_sock = accept( sock ,  NULL , NULL );
			if( client_sock > 0 ) addTask( server , responserRoutine , client_sock );
		}
	}
	{
		Task *cur = server->task_head;
		while( cur )
		{
			pthread_join( cur->handle , NULL );
			if( cur->client_socket ) close( cur->client_socket );
			Task *next = cur->next;
			free( cur );
			cur = next;
		}
	}
	close( sock );
	{
		Message *cur = server->msg_head;
		while( cur )
		{
			Message *next = cur->next;
			free( cur->msg );
			free( cur );
			cur = next;
		}
	}
	pthread_cond_destroy( &server->garbage_signal );
	pthread_mutex_destroy( &server->mutex );
	free( server );
#ifdef SHARED
	munmap( shared_mem , 0x100000 );
	shm_unlink( "shared.data" );
#endif
	return 0;
}