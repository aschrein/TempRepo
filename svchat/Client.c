#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include "MessageTypes.h"
int working = 1;
void release()
{
	working = 0;
}
int main( int argc , char **argv )
{
	signal( SIGINT , release );
	key_t key = ftok( "/shared_queue" , 0 );
	int msg_fd = msgget( key , S_IRUSR | S_IWUSR );
	int login = 1;
	if( msg_fd < 0 )
	{
		perror( "couldnot open message queue" );
	} else
	{
		MessageBlob blob;
		printf( "please enter user name( < 16 )\n" );
		scanf( "%s" , &blob.user_name );
		blob.type = LOGIN;
		msgsnd( msg_fd , &blob , sizeof( Message ) , 0 );
		blob.type = POST;
		while( working )
		{
			struct timeval timeval =
			{
				tv_sec : 0 ,
				tv_usec : 10 * 1000
			};
			fd_set s;
			FD_ZERO( &s );
			FD_SET( 0 , &s );
			int num = select( 1 , &s , NULL , NULL , &timeval );
			if( !working )
			{
				printf( "\n" );
				break;
			}
			if( FD_ISSET( 0 , &s ) )
			{
				int len = read( 0 , &blob.data[ 0 ] , 0x100 );
				if( len <= 0 )
				{
					break;
				}
				blob.data[ len - 1 ] = '\0';
				int res = msgsnd( msg_fd , &blob , sizeof( Message ) + len , 0 );
				if( res < 0 )
				{
					break;
				}
			}
		}
		blob.type = LOGOUT;
		msgsnd( msg_fd , &blob , sizeof( Message ) , 0 );
		close( msg_fd );
	}
	return 0;
}