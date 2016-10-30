#include "Modules.h"
#define _GNU_SOURCE
#include <link.h>
#include <dlfcn.h>
#include <stdint.h>
//ElfW( Sym )
void strcopy( char *dst , char const *src )
{
	while( *dst++ = *src++ );
}
void removeDependencies( ModuleSystem *system ,  Module *module )
{
	ModuleDependency *dep = system->dependency_head;
	while( dep )
	{
		if( dep->dst == module )
		{
			if( dep->prev )
			{
				ModuleDependency *next = dep->next;
				dep->prev->next = next;
				system->allocator->free( dep );
				dep = next;
				continue;
			} else
			{
				ModuleDependency *next = dep->next;
				system->dependency_head = next;
				system->allocator->free( dep );
				dep = next;
			}
		}
		dep = dep->next;
	}
}
void releaseDependantModules( ModuleSystem *system ,  Module *module )
{
	
	ModuleDependency *dep;
	dep_again:
	dep = system->dependency_head;
	while( dep )
	{
		if( dep->src == module )
		{
			releaseModule( system , dep->dst );
			goto dep_again;
		}
		dep = dep->next;
	}
}
void releaseModule( ModuleSystem *system ,  Module *module )
{
	dlclose( module->lib_handle );
	int i;
	if( module->prev )
	{
		module->prev->next = module->next;
	} else
	{
		system->modules_head = module->next;
	}
	removeDependencies( system , module );
	releaseDependantModules( system , module );
	/*for( i = 0; i < module->methods_count; i++ )
	{
		system->allocator->free( module->methods[ i ].arg_desc );
	}*/
	system->allocator->free( module->methods );
	system->allocator->free( module );
}
void loadDependency( char const *name , ModuleSystem *system )
{
	char full_name[ 0x100 ] = "modules/";
	int i;
	for( i = 0; i < strlen( name ); i++ )
	{
		full_name[ i + 8 ] = name[ i ];
	}
	int name_len = strlen( full_name );
	full_name[ name_len - 2 ] = 'd';
	full_name[ name_len - 1 ] = 'e';
	full_name[ name_len ] = 'p';
	full_name[ name_len + 1 ] = '\0';
	int dep_fd = open( name_len , O_RDONLY );
	if( dep_fd > 0 )
	{
		while( 1 )
		{
			char c;
			size_t read_len;
			int name_len = 0;
			while( ( read_len = read( dep_fd , &c , 1 ) ) && c != '\n' )
			{
				
				full_name[ i + 8 + name_len++ ] = c;
			}
			if( name_len )
			{
				full_name[ name_len ] = '.';
				full_name[ name_len + 1 ] = 's';
				full_name[ name_len + 2 ] = 'o';
				full_name[ name_len + 3 ] = '\0';
				loadModule( full_name , system );
			} else
			{
				break;
			}
		}
		close( dep_fd );
	}
}
int loadModule( char const *name , ModuleSystem *system )
{
	loadDependency( name , system );
	char full_name[ 0x100 ] = "modules/";
	int i;
	for( i = 0; i < strlen( name ); i++ )
	{
		full_name[ i + 8 ] = name[ i ];
	}
	
	void *lib = dlopen( full_name , RTLD_NOW | RTLD_GLOBAL );
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
	if( !hash_entry
	|| !string_table_size
	|| !string_table_entry
	|| !symbol_table_entry
	)
	{
		printf( "could not pull unavoidable structs from module %s\n" , name );
		dlclose( lib );
		return -1;
	}
	//printf( "symbols count: %p , strings count: %i\n" , symbol_table_size , string_table_size );
	//printf( "symbols entry: %p , strings entry: %p\n" , symbol_table_entry , string_table_entry );
	
	/*
	* extracting valid symbols through GNU_HASH
	* reference http://deroko.phearless.org/dt_gnu_hash.txt
	*/
	int export_symbols[ 0x100 ];
	int export_symbols_count = 0;
	{
		uint32_t *hash = ( uint32_t* )hash_entry;
		uint32_t num_buckets = hash[ 0 ];
		uint32_t symbol_bias = hash[ 1 ];
		uint32_t bitmask_nwords = hash[ 2 ];
		uint32_t *buckets = ( uint32_t* )(
			( uint8_t * )hash + 16 + bitmask_nwords * sizeof( size_t )
		);
		uint32_t *chains = buckets + num_buckets;
		
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
	}
	Module *out = ( Module* )system->allocator->alloc( sizeof( Module ) );
	out->lib_handle = lib;
	strcopy( out->name , name );
	out->methods_count = export_symbols_count;
	out->methods = ( Method* )system->allocator->alloc( sizeof( Method ) * export_symbols_count );
	out->next = NULL;
	out->prev = NULL;
	for( i = 0; i < export_symbols_count; i++ )
	{
		//printf( "symbol name:%s\n" , string_table_entry + symbol_table_entry[ export_symbols[ i ] ].st_name );
		strcopy( out->methods[ i ].name , string_table_entry + symbol_table_entry[ export_symbols[ i ] ].st_name );
		out->methods[ i ].ptr = dlsym( out->lib_handle , out->methods[ i ].name );
	}
	Module *m = system->modules_head;
	if( m == NULL )
	{
		system->modules_head = out;
	} else
	{
		while( m->next ) m = m->next;
		m->next = out;
		out->prev = m;
	}
	return 0;
	//symbol_table_size = ;
	
}
