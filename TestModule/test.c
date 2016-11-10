#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
int main( int argc , char * argv )
{
    char buffer[ 0x100 ] = { 0 };
	int fd = open( "/dev/MyDevice" , O_RDWR );
	//char buffer[ 0x100 ];
	printf( "opened char dev descriptor: %i\n" , fd );
	scanf( "%s" , buffer );
	write( fd , buffer , 0x100 );
	read( fd , buffer , 20 );
	printf( "%s\n" , buffer );
	return 0;
}
