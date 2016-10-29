#ifndef MODULES_H
#define MODULES_H
#include <stdint.h>
#include <stdlib.h>
typedef struct
{
	void ( *free )( void * );
	void ( *alloc )( size_t );
	void ( *realloc )( void * , size_t );
} Allocator;
typedef struct
{
	void *lib_handle;
	size_t string_table_size;
	char const *string_table_entry;
	Allocator allocator;
	void const *symbol_table_entry;
	uint32_t export_symbols_count;
	uint32_t export_symbols[];
} Module;
void releaseModule( Module * );
Module *loadModule( char const * , Allocator );
#endif
