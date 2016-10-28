#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#define _GNU_SOURCE
#include <link.h>
#include <dlfcn.h>
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
	ElfW( Sym ) const *symbol_table_entry;
	uint32_t export_symbols_count;
	uint32_t export_symbols[];
} Module;
void releaseModule( Module *module )
{
	dlclose( module->lib_handle );
	module->allocator.free( module );
}
Module *loadModule( char const *name , Allocator allocator )
{
	void *lib = dlopen( name , RTLD_NOW );
	if( !lib )
	{
		printf( "error loadding library\n" );
		return -1;
	}
	struct link_map *link_map;
	//2 means RTLD_DI_LINKMAP
	dlinfo( lib , 2 , &link_map );
	size_t string_table_size = 0;
	char const *string_table_entry = 0;
	ElfW( Sym ) const *symbol_table_entry = 0;
	void *hash_entry = 0;
	/*
	* pull nessesary structs
	* reference http://www.sco.com/developers/gabi/2000-07-17/contents.html
	*/
	{
		ElfW( Dyn ) const *dynamic_section_entry = link_map->l_ld;
		//printf( "dynamic section enty:%p\n" , dynamic_section_entry );
		while( dynamic_section_entry->d_tag != DT_NULL )
		{
			//printf( "tag found:%p\n" , dynamic_section_entry->d_tag );
			
			switch( dynamic_section_entry->d_tag )
			{
				case DT_STRTAB:
				{
					string_table_entry = ( char const * )
					( dynamic_section_entry->d_un.d_ptr );
				}
				break;
				case DT_SYMTAB:
				{
					symbol_table_entry = ( ElfW( Sym ) const * )
					( dynamic_section_entry->d_un.d_ptr );
				}
				break;
				//case DT_HASH:
				case DT_GNU_HASH:
				{
					hash_entry = dynamic_section_entry->d_un.d_ptr;
				}
				break;
				case DT_STRSZ:
				{
					string_table_size = dynamic_section_entry->d_un.d_val;
				}
				break;
				default:
				break;
			}
			dynamic_section_entry++;
		}
	}
	if( 0
	|| !string_table_size
	|| !string_table_entry
	|| !symbol_table_entry
	)
	{
		printf( "could not pull unavoidable structs from module %s\n" , name );
	}
	//printf( "symbols count: %p , strings count: %i\n" , symbol_table_size , string_table_size );
	//printf( "symbols entry: %p , strings entry: %p\n" , symbol_table_entry , string_table_entry );
	int i = 0;
	/*
	* extracting valid symbols through GNU_HASH
	* reference http://deroko.phearless.org/dt_gnu_hash.txt
	*/
	{
		uint32_t *hash = ( uint32_t* )hash_entry;
		uint32_t num_buckets = hash[ 0 ];
		uint32_t symbol_bias = hash[ 1 ];
		uint32_t bitmask_nwords = hash[ 2 ];
		uint32_t *buckets = ( uint32_t* )(
			( uint8_t * )hash + 16 + bitmask_nwords * sizeof( size_t )
		);
		uint32_t *chains = buckets + num_buckets;
		int export_symbols[ 0x100 ];
		int export_symbols_count = 0;
		int chain_index = 0;
		for( i = 0; i < num_buckets; i++ )
		{
			if( buckets[ i ] == 0 )
			{
				continue;
			}
			uint32_t sym_index = buckets[ i ];
			int offset = 0;
			while( 1 )
			{
				//printf( "name index:%i " , symbol_table_entry[ i ].st_name );
				//printf( "name:%s\n" , string_table_entry + symbol_table_entry[ sym_index + offset ].st_name );
				//only take into account such names that does not contain '_" at the beginning
				if( *( string_table_entry + symbol_table_entry[ sym_index + offset ].st_name ) != '_' )
				{
					export_symbols[ export_symbols_count++ ] = sym_index + offset;
				}
				chain_index++;
				offset++;
				if( chains[ chain_index ] & 1 )
				{
					break;
				}
			}
		}
		for( i = 0; i < export_symbols_count; i++ )
		{
			printf( "symbol name:%s\n" , string_table_entry + symbol_table_entry[ export_symbols[ i ] ].st_name );
		}
	}
	//symbol_table_size = ;
	
}
int main( int argc , char **argv )
{
	/*Display *d = XOpenDisplay( NULL );
	if( d == NULL )
	{
		printf( "Cannot open display\n" );
		exit( 1 );
	}

	int s = DefaultScreen( d );
	Window w = XCreateSimpleWindow( d , RootWindow( d , s ) , 10 , 10 , 512 , 512 , 1 , BlackPixel( d , s ) , WhitePixel( d , s ) );
	XSelectInput( d , w , ExposureMask | KeyPressMask );
	XMapWindow( d , w );*/
	Allocator allocator =
	{
		.free = free ,
		.alloc = malloc ,
		.realloc = realloc
	};
	Module *module = loadModule( "plugins/add.so" , allocator );
	////
	/*while( 1 )
	{
		XEvent e;
		XNextEvent( d , &e );
		if( e.type == Expose )
		{
			XFillRectangle( d , w , DefaultGC( d , s ) , 20 , 20 , 10 , 10 );
			XDrawString( d , w , DefaultGC( d , s ) , 10 , 50 , "hello!" , 6 );
		}
		if (e.type == KeyPress)
		break;
	}
	XCloseDisplay(d);*/
   return 0;
}
/*int main( int argc , char **argv )
{
	Complex c = { 1.0f , 0.0f };
	printComplex( c );
	Complex a = { 0.0f , 2.0f };
	printComplex( div( c , a ) );
	
	printComplexExponent( expComplex(
		( Complex ){ 1.0f , 1.0f }
	) );//1.414214 * e^i0.785398

	printComplexExponent( expComplex(
		mul( ( Complex ){ 1.0f , 1.0f } , ( Complex ){ 1.0f , -1.0f } )
	) );//2.000000 * e^i0.000000
	printComplex( divr( ( Complex ){ 1.0f , 1.0f } , 0 ) );//{ r: NAN i: NAN }
	printComplex( mul( ( Complex ){ 1.0f , 1.0f } , ( Complex ){ 1.0f , -1.0f } ) );//{ r: 2.000000 i: 0.000000 }
	printComplex( div( ( Complex ){ 1.0f , 1.0f } , ( Complex ){ 1.0f , -1.0f } ) );//{ r: 0.000000 i: 1.000000 }
	printComplex( sub( ( Complex ){ 1.0f , 1.0f } , ( Complex ){ 1.0f , -1.0f } ) );//{ r: 0.000000 i: 2.000000 }
	printComplex( add( ( Complex ){ 1.0f , 1.0f } , ( Complex ){ 1.0f , -1.0f } ) );//{ r: 2.000000 i: 0.000000 }
	printf( "%f\n" , mod( ( Complex ){ 3.0f , 4.0f } ) );//5
	printComplex( neg( ( Complex ){ 3.0f , 1.0f } ) );//{ r: -3.000000 i: -1.000000 }
	printComplex( conjugate( ( Complex ){ 3.0f , 1.0f } ) );//{ r: 3.000000 i: -1.000000 }
	
	return 0;
}*/
