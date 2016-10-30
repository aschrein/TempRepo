#ifndef ALLOCATOR_H
#define ALLOCATOR_H
#include <stdlib.h>
typedef struct
{
	void ( *free )( void * );
	void *( *alloc )( size_t );
	void *( *realloc )( void * , size_t );
} Allocator;
#endif
