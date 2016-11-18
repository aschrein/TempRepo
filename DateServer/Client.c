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
	int ctype = 0;
	int sock;
	printf( "please enter connection type: 0 - tcp , 1 - udp\n" );
	scanf( "%i" , &ctype );
	struct sockaddr_in sa;
	if( ctype == 0 )
	{
		sa =
		( struct sockaddr_in ){
			sin_family : AF_INET ,
			sin_port : htons( 8080 ) ,
			sin_addr : { s_addr : inet_addr( "127.0.0.1" ) }
		};
		sock = socket( AF_INET , SOCK_STREAM , 0 );
		if( connect( sock , ( struct sockaddr* )&sa , sizeof( sa ) ) )
		{
			perror( "couldnot connect to server" );
			return -1;
		}
	} else if( ctype == 1 )
	{
		sa =
		( struct sockaddr_in ){
			sin_family : AF_INET ,
			sin_port : htons( 8081 ) ,
			sin_addr : { s_addr : inet_addr( "127.0.0.1" ) }
		};
		sock = socket( AF_INET , SOCK_DGRAM , IPPROTO_UDP );
	} else
	{
		printf( "invalid connection type\n" );
	}
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
			FD_SET( sock , &s );
			int num = select( sock + 1 , &s , NULL , NULL , &timeval );
			if( !working )
			{
				printf( "\n" );
				break;
			}
			if( FD_ISSET( 0 , &s ) )
			{
				int len = read( 0 , buffer , 0x100 );
				if( len <= 0 )
				{
					break;
				}
				buffer[ len - 1 ] = 0;
				if( ctype == 0 )
				{
					send( sock , buffer , len , 0 );
				} else
				{
					sendto( sock , buffer , len , 0 , ( struct sockaddr * )&sa , sizeof( sa ) );
				}
			}
			if( FD_ISSET( sock , &s ) )
			{
				int len;
				if( ctype == 0 )
				{
					len = recv( sock , buffer , 0x100 , 0 );
				} else
				{
					struct sockaddr_in sin;
					socklen_t slen;
					len = recvfrom( sock , buffer , 0x100 , 0 , ( struct sockaddr * )&sin , &slen );
				}
				if( len <= 0 )
				{
					break;
				}
				//buffer[ len - 1 ] = '\n';
				buffer[ len ] = '\0';
				printf( "%s\n" , buffer );
			}
		}
	}
	close( sock );
	return 0;
}