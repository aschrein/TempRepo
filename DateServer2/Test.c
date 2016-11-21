#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <poll.h>
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
	if( sock <= 0 )
	{
		perror( "error creating socket\n" );
		return -1;
	}
    {
		char buffer[ 0x100 + 1 ];
		struct pollfd pollfds[ 2 ];
		while( working )
		{
			int req_mask = POLLIN | POLLPRI;
			pollfds[ 0 ] = ( struct pollfd )
			{
				fd: sock ,
				events: req_mask ,
				revents: 0
			};
			int num = poll( pollfds , 1 , 100 );
			if( !working )
			{
				printf( "\n" );
				break;
			}
            if( pollfds[ 0 ].revents & req_mask )
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
				buffer[ len ] = '\0';
				printf( "%s\n" , buffer );
			}
            if( ctype == 0 )
            {
                send( sock , "get_date" , 9 , 0 );
            } else
            {
                int err = sendto( sock , "get_date" , 9 , 0 , ( struct sockaddr * )&sa , sizeof( sa ) );
                if( err <= 0 )
                {
                    printf( "error sending udp data\n" );
                }
            }

		}
	}
	close( sock );
	return 0;
}
