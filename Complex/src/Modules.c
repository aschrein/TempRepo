#include "Modules.h"
#define _GNU_SOURCE
#include <link.h>
#include <dlfcn.h>
#include <stdint.h>

//ElfW( Sym )
void releaseModule( Module *module )
{
	dlclose( module->lib_handle );
	module->allocator.free( module );
}
Module *loadModule( char const *name , Allocator allocator )
{
	void *lib = dlopen( name , RTLD_NOW | RTLD_GLOBAL );
	if( !lib )
	{
		printf( "error loadding library:%s\n" , dlerror() );
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
