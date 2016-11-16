#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "SharedMessages.h"
#include <sys/signal.h>
#include <sys/select.h>
int working = 1;
void release()
{
	working = 0;
}
int main( int argc , char **argv )
{
	signal( SIGINT , release );
	int shared_fd = shm_open( "shared.data" , O_RDWR , 0 );
	if( shared_fd > 0 )
	{
		MessageBoard *msg_board = mmap( NULL , SHARED_MEM_SIZE , PROT_READ | PROT_WRITE , MAP_SHARED , shared_fd , 0 );
		close( shared_fd );
		int usr_id = addUser( msg_board );
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
				addMessage( msg_board , usr_id , buffer );
			}
		}
		removeUser( msg_board , usr_id );
		munmap( msg_board , SHARED_MEM_SIZE );
	} else
	{
		perror( "failed to open shared mem file" );
	}
	return 0;
}