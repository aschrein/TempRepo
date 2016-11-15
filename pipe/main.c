#include <unistd.h>
int main( int argc , char **argv )
{
	int pipefd[ 2 ];
	pipe( pipefd );
	int chpid = fork();
	if( !chpid )
	{
		close( pipefd[ 1 ] );
		dup2( pipefd[ 0 ] , 0 );
		system ( "grep *.c" );
	} else if( chpid > 0 )
	{
		close( pipefd[ 0 ] );
		int chpid1 = fork();
		if( !chpid1 )
		{
			dup2( pipefd[ 1 ] , 1 );
			system ( "ls" );
		}
		close( pipefd[ 1 ] );
		waitpid( chpid , NULL , 0 );
		waitpid( chpid1 , NULL , 0 );
	}
    return 0;
}
/*char const * const argv[] =
		{
			"grep" ,
			"*.out" ,
			NULL
		};
		execv( "grep" , argv );*/