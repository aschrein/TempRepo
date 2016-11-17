#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/signal.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <sys/msg.h>
#include <sys/types.h>
#define BUFFER_WIDTH 0x100
#include "MessageTypes.h"
#include <stdint.h>
int working = 1;
void release()
{
	working = 0;
}
typedef struct
{
	char raw[ 0x10 ];
} UserName;
UserName users[ 0x100 ];
int users_count = 0;
typedef struct MessageNode_t
{
	struct MessageNode_t *next;
	Message msg;
} MessageNode;
int main( int argc , char **argv )
{
	signal( SIGINT , release );
	key_t key = ftok( "/shared_queue" , 0 );
	int msg_fd = msgget( key , IPC_CREAT | S_IRUSR | S_IWUSR );
	if( msg_fd < 0 )
	{
		perror( "error while opening shared message queue" );
		return -1;
	}
	/*{
		struct msqid_ds msqid_ds;
		msgctl( msg_fd , IPC_STAT , &msqid_ds );
		msqid_ds.msg_qbytes = MAX_MESSAGE_LEN;
		msgctl( msg_fd , IPC_SET , &msqid_ds );
	}*/
	{
		initscr();
		raw();
		noecho();
		keypad( stdscr , TRUE );
		nodelay( stdscr , 1 );
	}
	MessageNode *msg_head = NULL;
	MessageBlob blob;
	while( working )
	{
		int len = msgrcv( msg_fd , &blob , sizeof( MessageBlob ) , 0 , IPC_NOWAIT | MSG_NOERROR );
		if( len > 0 )
		{
			if( blob.type == LOGIN )
			{
				int i = 0;
				for(; i < users_count; i++ )
				{
					if( users[ i ].raw[ 0 ] == 0 )
					{
						strcpy( users[ i ].raw , blob.user_name );
						i = -1;
						break;
					}
				}
				if( i >= 0 )
				{
					strcpy( users[ users_count++ ].raw , blob.user_name );
				}
			} else if( blob.type == LOGOUT )
			{
				int i = 0;
				for(; i < users_count; i++ )
				{
					if( !strcmp( users[ i ].raw , blob.user_name ) )
					{
						users[ i ].raw[ 0 ] = 0;
						break;
					}
				}
				while( users_count && users[ users_count - 1 ].raw[ 0 ] == 0 ) users_count--;
			} else if( blob.type == POST )
			{
				MessageNode *node = ( Message* )malloc( len + sizeof( MessageNode* ) );
				memcpy( &node->msg , &blob , len );
				node->next = NULL;
				if( msg_head )
				{
					MessageNode *cur = msg_head;
					while( cur->next )
					{
						cur = cur->next;
					}
					cur->next = node;
				} else
				{
					msg_head = node;
				}
			}
		}
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
			{
				y = 3;
				int i = 0;
				for(; i < users_count; i++ )
				{
					if( users[ i ].raw[ i ] != 0 ) mvprintw( y++ , 0 , "%s" , users[ i ].raw );
				}
			}
			{
				y = 3;
				MessageNode *cur = msg_head;
				int offset = 0;
				while( cur )
				{
					offset++;
					cur = cur->next;
				}
				offset = height - offset - 3;
				cur = msg_head;
				while( cur )
				{
					if( offset > 0 ) mvprintw( y++ , 11 , "%s:%s" , cur->msg.user_name , cur->msg.msg );
					offset++;
					cur = cur->next;
				}
			}
		}
		refresh();
		usleep( 1000 );
	}
	{
		endwin();
		printf( "exited normalliy\n" );
		MessageNode *cur = msg_head;
		while( cur )
		{
			MessageNode *next = cur->next;
			free( cur );
			cur = next;
		}
		msgctl( msg_fd , IPC_RMID , 0 );
		close( msg_fd );
	}
	return 0;
}