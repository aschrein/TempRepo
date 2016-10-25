#include <stdio.h>
char char_array[ 10 ] = { 'A' , 1 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 };
char char_array1[ 10 ] = { 'A' , 0 , 0 , 0 , 0x01 , 0xff , 0xff , 0xff , 0 , 0 };
char char_array2[ 10 ] = { 0x01 , 0xff , 0xff , 0xff , 'B' , 0 , 0 , 0 , 0 , 0 };
typedef struct
{
	char a;
	int b;
} Test;
typedef struct
{
	int b;
	char a;
} Test2;
int main( int argc , char **argv )
{
	Test *t = ( Test* )( void* )0;
	printf( "%i %i\n" , &t->a , &t->b );
	t = ( Test* )( void * )char_array;
	printf( "%c %i\n" , t->a , t->b );
	t = ( Test* )( void * )char_array1;
	printf( "%c %i\n" , t->a , t->b );
	Test2 *t2 = ( Test2* )( void * )char_array2;
	printf( "%c %i\n" , t2->a , t2->b );
	return 0;
}
