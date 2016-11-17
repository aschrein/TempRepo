#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
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
	key_t key = ftok( "/shared_mem" , 0 );
	int shared_fd = shmget( key , SHARED_MEM_SIZE , IPC_CREAT | S_IRUSR | S_IWUSR );
	if( shared_fd > 0 )
	{
		MessageBoard *msg_board = shmat( shared_fd , NULL , 0 );
		
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
		shmdt( msg_board );
		close( shared_fd );
	} else
	{
		perror( "failed to open shared mem file" );
	}
	return 0;
}