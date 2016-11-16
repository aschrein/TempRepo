#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/signal.h>
int working = 1;
void release()
{
	working = 0;
}
int main( int argc , char **argv )
{
	signal( SIGINT , release );
	int sock = socket( AF_INET , SOCK_STREAM , 0 );
	struct sockaddr_in sa =
	{
		sin_family : AF_INET ,
		sin_port : htons( 8080 ) ,
		sin_addr : { s_addr : inet_addr( "127.0.0.1" ) }
	};
	if( connect( sock , ( struct sockaddr* )&sa , sizeof( sa ) ) )
	{
		perror( "couldnot connect to server" );
	} else
	{
		char buffer[ 0x100 + 1 ];
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
				int len = read( 0 , buffer , 0x100 );
				buffer[ len ] = 0;
				send( sock , buffer , len , 0 );
			}
		}
	}
	close( sock );
	return 0;
}