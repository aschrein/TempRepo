#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
int main( int argc , char **argv )
{
	int shared_fd = shm_open( "shared.data" , O_RDWR , 0 );
	if( shared_fd > 0 )
	{
		void *shared_mem = mmap( NULL , 0x100000 , PROT_READ | PROT_WRITE , MAP_SHARED , shared_fd , 0 );
		close( shared_fd );
		printf( "shared file opened and contans %i\n" , ( ( int* )shared_mem )[ 0 ] );
		munmap( shared_mem , 0x100000 );
		
	} else
	{
		perror( "failed to open shared mem file" );
	}
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
		while( 1 )
		{
			int len = read( 0 , buffer , 0x100 );
			if( !len )
			{
				break;
			}
			send( sock , buffer , len , 0 );
		}
		//int len = recv( sock , buffer , 0x100 , 0 );
		//buffer[ 0x100 ] = 0;
		//printf( "recv:%s\n" , buffer );
	}
	close( sock );
	return 0;
}