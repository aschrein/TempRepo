#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ncurses.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/signal.h>
#include <poll.h>
#include <sys/epoll.h>
#include <fcntl.h>
int working = 1;
void release()
{
	working = 0;
}
int tcp_sock , udp_sock;
int tcp_clients[ 0x100 ];
int tcp_clients_count;
void insertTCPUser( int sock )
{
	int i;
	for( i = 0; i < tcp_clients_count; i++ )
	{
		if( tcp_clients[ i ] < 0 )
		{
			tcp_clients[ i ] = sock;
			return;
		}
	}
	tcp_clients[ tcp_clients_count++ ] = sock;
}
void removeTCPUser( int sock )
{
	int i;
	for( i = 0; i < tcp_clients_count; i++ )
	{
		if( tcp_clients[ i ] == sock )
		{
			close( tcp_clients[ i ] );
			tcp_clients[ i ] = -1;
			break;
		}
	}
	while( tcp_clients_count && tcp_clients[ tcp_clients_count - 1 ] == -1 ) tcp_clients_count--;
}
int initSockets()
{
	tcp_sock = socket( AF_INET , SOCK_STREAM | SOCK_NONBLOCK , 0 );
	if( tcp_sock < 0 )
	{
		perror( "could not create tcp socket" );
		return -1;
	}
	{
		struct sockaddr_in sa =
		{
			sin_family : AF_INET ,
			sin_port : htons( 8080 ) ,
			sin_addr : { s_addr : htonl( INADDR_ANY ) }
		};
		bind( tcp_sock , ( struct sockaddr* )&sa , sizeof( sa ) );
		listen( tcp_sock , 0x10 );
	}
	udp_sock = socket( AF_INET , SOCK_DGRAM | SOCK_NONBLOCK , IPPROTO_UDP );
	if( udp_sock < 0 )
	{
		close( tcp_sock );
		perror( "could not create udp socket" );
		return -1;
	}
	{
		struct sockaddr_in sa =
		{
			sin_family : AF_INET ,
			sin_port : htons( 8081 ) ,
			sin_addr : { s_addr : htonl( INADDR_ANY ) }
		};
		bind( udp_sock , ( struct sockaddr* )&sa , sizeof( sa ) );
	}
}
void fDate( char *buf )
{
	//http://stackoverflow.com/questions/1442116/how-to-get-date-and-time-value-in-c-program
	time_t t = time( NULL );
	struct tm tm = *localtime( &t );
	sprintf( buf , "%d-%d-%d %d:%d:%d", tm.tm_year + 1900 ,
	tm.tm_mon + 1, tm.tm_mday , tm.tm_hour , tm.tm_min , tm.tm_sec );
}
int main( int argc , char **argv )
{
	signal( SIGINT , release );
	if( initSockets() )
	{
		return -1;
	}
	int i;
	printf( "supported commands:'get_date','get_users_count':tcp clients count\n" );
	//https://banu.com/blog/2/how-to-use-epoll-a-complete-example-in-c/
	//http://man7.org/linux/man-pages/man7/epoll.7.html
	//http://man7.org/linux/man-pages/man2/epoll_create.2.html
	int epollfd = epoll_create( 10 );
	struct epoll_event event;
	struct epoll_event *events = ( struct epoll_event* )malloc( sizeof( struct epoll_event ) * ( 0x100 * 2 ) );
	memset( events , 0 , sizeof( struct epoll_event ) * ( 0x100 * 2 ) );
	int req_mask = POLLIN | EPOLLET;
	{
		//http://man7.org/linux/man-pages/man2/epoll_ctl.2.html
		event.data.fd = tcp_sock;
		event.events = req_mask;
		epoll_ctl( epollfd , EPOLL_CTL_ADD , tcp_sock , &event );
		event.data.fd = udp_sock;
		event.events = req_mask;
		epoll_ctl( epollfd , EPOLL_CTL_ADD , udp_sock , &event );
	}
	while( working )
	{
		int real_users_count = 0;
		for( i = 0; i < tcp_clients_count; i++ )
		{
			if( tcp_clients[ i ] > 0 )
			{
				real_users_count++;
			}
		}
		//http://man7.org/linux/man-pages/man2/epoll_wait.2.html
		int num = epoll_wait( epollfd , events , 0x100 * 2 , 100 );
		/*__________*/
		if( !working )
		{
			break;
		}
		int j;
		for( j = 0; j < num; j++ )
		{
			if( events[ j ].data.fd == tcp_sock )
			{
				int client_sock = accept( tcp_sock ,  NULL , NULL );
				if( client_sock > 0 )
				{
					int flags = fcntl ( client_sock , F_GETFL , 0 );
					flags |= O_NONBLOCK;
					fcntl( client_sock , F_SETFL , flags );
					insertTCPUser( client_sock );
					event.data.fd = client_sock;
					event.events = req_mask;
					epoll_ctl( epollfd , EPOLL_CTL_ADD , client_sock , &event );
					printf( "tcp client connected\n" );
				}
			}
			char buf[ 0x100 ];
			if( events[ j ].data.fd == udp_sock )
			{
				struct sockaddr_in sa;
				socklen_t slen;
				int len = recvfrom( udp_sock , buf , 0x100 , 0 , ( struct sockaddr * )&sa , &slen );
				if( len > 0 )
				{
					if( strcmp( buf , "get_date" ) == 0 )
					{
						fDate( buf );
						sendto( udp_sock , buf , strlen( buf ) , 0 , ( struct sockaddr * )&sa , slen );
					} else if( strcmp( buf , "get_users_count" ) == 0 )
					{
						sprintf( buf , "tcp clients count:%i" , real_users_count );
						sendto( udp_sock , buf , strlen( buf ) , 0 , ( struct sockaddr * )&sa , slen );
					} else
					{
						sendto( udp_sock , "wrong command" , 14 , 0 , ( struct sockaddr * )&sa , slen );
					}
				}
			}
			for( i = 0; i < tcp_clients_count; i++ )
			{
				if( tcp_clients[ i ] > 0 && events[ j ].data.fd == tcp_clients[ i ] )
				{
					int len = recv( tcp_clients[ i ] , buf , 0x100 , 0 );
					if( len <= 0 )
					{
						epoll_ctl( epollfd , EPOLL_CTL_DEL , tcp_clients[ i ] , NULL );
						removeTCPUser( tcp_clients[ i ] );
						printf( "tcp client disconnected\n" );
						continue;
					}
					if( strcmp( buf , "get_date" ) == 0 )
					{
						fDate( buf );
						send( tcp_clients[ i ] , buf , strlen( buf ) , 0 );
					} else if( strcmp( buf , "get_users_count" ) == 0 )
					{
						sprintf( buf , "tcp clients count:%i" , real_users_count );
						send( tcp_clients[ i ] , buf , strlen( buf ) , 0 );
					} else
					{
						send( tcp_clients[ i ] , "wrong command" , 14 , 0 );
					}
				}
			}
		}
	}
	free( events );
	close( epollfd );
	printf( "exited normalliy\n" );
	for( i = 0; i < tcp_clients_count; i++ )
	{
		if( tcp_clients[ i ] > 0 )
		{
			close( tcp_clients[ i ] );
		}
	}
	close( tcp_sock );
	close( udp_sock );
	
	return 0;
}