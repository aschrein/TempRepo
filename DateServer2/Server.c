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
#include <pthread.h>
#define MAX_CLIENTS 0x100
#define MAX_MSG_SIZE 0x100
int req_mask = POLLIN | EPOLLET;
typedef struct Command_t
{
	int client_fd;
	int udp;
	struct Command_t *next;
} Command;
typedef struct Worker_t
{
	struct Worker_t *next;
	struct Server_t *server;
	pthread_t handle;
} Worker;
typedef struct Server_t
{
	int tcp_sock , udp_sock , epollfd;
	int tcp_clients[ MAX_CLIENTS ];
	int tcp_clients_count , real_tcp_clients_count;
	Command *cmd_head;
	Worker *worker_head;
	pthread_mutex_t mutex;
	pthread_cond_t cmd_signal;
	int working;
} Server;
Server *server;
void release()
{
	server->working = 0;
}
void responserRoutine( Worker *worker )
{
	char buf[ MAX_MSG_SIZE ];
	while( worker->server->working )
	{
		Command *cmd = NULL;
		pthread_mutex_lock( &worker->server->mutex );
		cmd = worker->server->cmd_head;
		if( !cmd )
		{
			pthread_cond_wait( &worker->server->cmd_signal , &worker->server->mutex );
			pthread_mutex_unlock( &worker->server->mutex );
			continue;
		} else
		{
			worker->server->cmd_head = cmd->next;
		}
		pthread_mutex_unlock( &worker->server->mutex );
		printf( "worker%p woke up\n" , worker );
		if( cmd )
		{
			if( !cmd->udp )
			{
				int len = recv( cmd->client_fd , buf , MAX_MSG_SIZE , 0 );
				if( len <= 0 )
				{
					removeTCPUser( worker->server , cmd->client_fd );
					printf( "tcp client disconnected\n" );
				} else
				{
					if( strcmp( buf , "get_date" ) == 0 )
					{
						fDate( buf );
						send( cmd->client_fd , buf , strlen( buf ) , 0 );
					} else if( strcmp( buf , "get_users_count" ) == 0 )
					{
						sprintf( buf , "tcp clients count:%i" , worker->server->real_tcp_clients_count );
						send( cmd->client_fd , buf , strlen( buf ) , 0 );
					} else
					{
						send( cmd->client_fd , "wrong command" , 14 , 0 );
					}
				}
			} else
			{
				struct sockaddr_in sa;
				socklen_t slen;
				int len;
				do
				{
					len = recvfrom( server->udp_sock , buf , 0x100 , 0 , ( struct sockaddr * )&sa , &slen );
					if( len > 0 )
					{
						if( strcmp( buf , "get_date" ) == 0 )
						{
							fDate( buf );
							sendto( server->udp_sock , buf , strlen( buf ) , 0 , ( struct sockaddr * )&sa , slen );
						} else if( strcmp( buf , "get_users_count" ) == 0 )
						{
							sprintf( buf , "tcp clients count:%i" , server->real_tcp_clients_count );
							sendto( server->udp_sock , buf , strlen( buf ) , 0 , ( struct sockaddr * )&sa , slen );
						} else
						{
							sendto( server->udp_sock , "wrong command" , 14 , 0 , ( struct sockaddr * )&sa , slen );
						}
					}
				} while( len > 0 );
			}
			free( cmd );
		}
	}
}
void insertTCPUser( Server *server , int client_sock )
{
	int flags = fcntl ( client_sock , F_GETFL , 0 );
	flags |= O_NONBLOCK;
	fcntl( client_sock , F_SETFL , flags );
	struct epoll_event event;
	event.data.fd = client_sock;
	event.events = req_mask;
	epoll_ctl( server->epollfd , EPOLL_CTL_ADD , client_sock , &event );
	pthread_mutex_lock( &server->mutex );
	int i;
	for( i = 0; i < server->tcp_clients_count; i++ )
	{
		if( server->tcp_clients[ i ] < 0 )
		{
			server->tcp_clients[ i ] = client_sock;
			pthread_mutex_unlock( &server->mutex );
			return;
		}
	}
	server->tcp_clients[ server->tcp_clients_count++ ] = client_sock;
	pthread_mutex_unlock( &server->mutex );
}
void removeTCPUser( Server *server , int sock )
{
	pthread_mutex_lock( &server->mutex );
	int i;
	for( i = 0; i < server->tcp_clients_count; i++ )
	{
		if( server->tcp_clients[ i ] == sock )
		{
			close( server->tcp_clients[ i ] );
			epoll_ctl( server->epollfd , EPOLL_CTL_DEL , sock , NULL );
			server->tcp_clients[ i ] = -1;

			break;
		}
	}
	while( server->tcp_clients_count && server->tcp_clients[ server->tcp_clients_count - 1 ] == -1 ) server->tcp_clients_count--;
	pthread_mutex_unlock( &server->mutex );
}
int iniServer( Server *server , int num_workers )
{
	server->tcp_sock = socket( AF_INET , SOCK_STREAM | SOCK_NONBLOCK , 0 );
	if( server->tcp_sock < 0 )
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
		bind( server->tcp_sock , ( struct sockaddr* )&sa , sizeof( sa ) );
		listen( server->tcp_sock , 0x10 );
	}
	server->udp_sock = socket( AF_INET , SOCK_DGRAM | SOCK_NONBLOCK , IPPROTO_UDP );
	if( server->udp_sock < 0 )
	{
		close( server->tcp_sock );
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
		bind( server->udp_sock , ( struct sockaddr* )&sa , sizeof( sa ) );
	}
	server->working = 1;
	pthread_mutex_init( &server->mutex , NULL );
	pthread_cond_init( &server->cmd_signal , NULL );
	server->epollfd = epoll_create( 0x100 );
	struct epoll_event event;
	event.data.fd = server->tcp_sock;
	event.events = req_mask;
	epoll_ctl( server->epollfd , EPOLL_CTL_ADD , server->tcp_sock , &event );
	event.data.fd = server->udp_sock;
	event.events = req_mask;
	epoll_ctl( server->epollfd , EPOLL_CTL_ADD , server->udp_sock , &event );
	int i;
	Worker *cur = NULL;
	for( i = 0; i < num_workers; i++ )
	{
		Worker *worker = ( Worker* )malloc( sizeof( Worker ) );
		memset( worker , 0 , sizeof( Worker ) );
		worker->server = server;
		pthread_create( &worker->handle , NULL , responserRoutine , worker );
		if( !cur )
		{
			server->worker_head = worker;
			cur = worker;
		} else
		{
			cur->next = worker;
			cur = worker;
		}
	}
	return 0;
}
void releaseServer( Server *server )
{
	server->working = 0;
	pthread_cond_broadcast( &server->cmd_signal );
	Worker *cur = server->worker_head;
	while( cur )
	{
		Worker *next = cur->next;

		pthread_join( cur->handle , NULL );
		free( cur );
		cur = next;
	}
	close( server->epollfd );
	pthread_mutex_destroy( &server->mutex );
	pthread_cond_destroy( &server->cmd_signal );
	int i;
	for( i = 0; i < server->tcp_clients_count; i++ )
	{
		if( server->tcp_clients[ i ] > 0 )
		{
			close( server->tcp_clients[ i ] );
		}
	}
	close( server->tcp_sock );
	close( server->udp_sock );
	printf( "exited normalliy\n" );
}
void loopServer( Server *server )
{
	struct epoll_event event;
	struct epoll_event *events = ( struct epoll_event* )malloc( sizeof( struct epoll_event ) * ( 0x100 + 2 ) );
	memset( events , 0 , sizeof( struct epoll_event ) * ( 0x100 + 2 ) );
	int i;
	while( server->working )
	{
		server->real_tcp_clients_count = 0;
		for( i = 0; i < server->tcp_clients_count; i++ )
		{
			if( server->tcp_clients[ i ] > 0 )
			{
				server->real_tcp_clients_count++;
			}
		}
		int num = epoll_wait( server->epollfd , events , 0x100 + 2 , 100 );
		/*__________*/
		if( !server->working )
		{
			break;
		}
		int j;
		for( j = 0; j < num; j++ )
		{
			if( events[ j ].data.fd == server->tcp_sock )
			{
				int client_sock = accept( server->tcp_sock ,  NULL , NULL );
				if( client_sock > 0 )
				{
					insertTCPUser( server , client_sock );
					printf( "tcp client connected\n" );
				}
			}
			for( i = 0; i < server->tcp_clients_count; i++ )
			{
				if( server->tcp_clients[ i ] > 0 && events[ j ].data.fd == server->tcp_clients[ i ] )
				{
					Command *cmd = ( Command* )malloc( sizeof( Command ) );
					cmd->next = NULL;
					cmd->client_fd = server->tcp_clients[ i ];
					cmd->udp = 0;
					pthread_mutex_lock( &server->mutex );
					Command *cur = server->cmd_head;
					if( cur )
					{
						while( cur->next ) cur = cur->next;
						cur->next = cmd;
					} else
					{
						server->cmd_head = cmd;
					}
					pthread_cond_signal( &server->cmd_signal );
					pthread_mutex_unlock( &server->mutex );
				}
			}
			char buf[ 0x100 ];
			if( events[ j ].data.fd == server->udp_sock )
			{
				Command *cmd = ( Command* )malloc( sizeof( Command ) );
				cmd->next = NULL;
				cmd->client_fd = server->udp_sock;
				pthread_mutex_lock( &server->mutex );
				cmd->udp = 1;
				Command *cur = server->cmd_head;
				if( cur )
				{
					while( cur->next ) cur = cur->next;
					cur->next = cmd;
				} else
				{
					server->cmd_head = cmd;
				}
				pthread_cond_signal( &server->cmd_signal );
				pthread_mutex_unlock( &server->mutex );
			}
		}
	}
	printf( "server escaped loop\n" );
	free( events );
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
	server = ( Server* )malloc( sizeof( Server ) );
	if( iniServer( server , 4 ) )
	{
		free( server );
		return -1;
	}
	int i;
	printf( "supported commands:'get_date','get_users_count':tcp clients count\n" );
	loopServer( server );
	releaseServer( server );
	return 0;
}
