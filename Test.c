#include <malloc.h>
#include <stdio.h>
#include "DummyList.h"
static int ints[ 10 ] = { 0 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 , 9 };
Flow printInt( void *ptr )
{
	printf( "%i\n" , *( int* )ptr );
	return CONTINUE;
}
static int allocation_counter = 0;
void *customMalloc( size_t size )
{
	void *out = malloc( size );
	if( out != NULL )
	{
		allocation_counter += 1;
	}
	return out;
}
void customFree( void *ptr )
{
	if( ptr != NULL )
	{
		allocation_counter -= 1;
		free( ptr );
	}
}
int main( int argc , char **argv )
{
	List *list;
	Allocator allocator =
	{
		.alloc = customMalloc ,
		.free = customFree
	};
	Error err = createList( &list , allocator );
	if( err != SUCCESS )
	{
		printf( "couldnot crete list with error: %i\n" , err );
	}
	int i;
	for( i = 0; i < 10; i++ )
	{
		err = appendBack( list , ints + i );
		if( err != SUCCESS )
		{
			printf( "error while appending %ith element\n" , i );
		}
	}
	printf( "allocated blocks: %i\n" , allocation_counter );
	printf( "list length: %i\n" , list->length );
	forEach( list , printInt );
	for( i = 0; i < 5; i++ )
	{
		err = removeFirstWithContent( list , ints + i );
		if( err != SUCCESS )
		{
			printf( "error while removing %ith element\n" , i );
		}
	}
	for( i = 9; i >= 5; i-- )
	{
		err = removeFirstWithContent( list , ints + i );
		if( err != SUCCESS )
		{
			printf( "error while removing %ith element\n" , i );
		}
	}
	printf( "list length: %i\n" , list->length );
	err = destroyList( &list );
	printf( "allocated blocks: %i\n" , allocation_counter );
	return 0;
}
