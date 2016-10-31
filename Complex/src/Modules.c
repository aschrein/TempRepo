#include "Modules.h"
#define _GNU_SOURCE
#include <link.h>
#include <dlfcn.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#define DEBUG
#ifdef DEBUG
#define LOG( ... ) printf( __VA_ARGS__ )
#else
#define LOG( ... )
#endif
//ElfW( Sym )
static inline void strcopy( char *dst , char const *src )
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
			LOG( "removing dependency %s->%s\n" , dep->src , dep->dst );
			if( dep->prev )
			{
				ModuleDependency *next = dep->next;
				dep->prev->next = next;
				if( next )
				{
					next->prev = dep->prev;
				}
				system->allocator->free( dep );
				dep = next;
				continue;
			} else
			{
				ModuleDependency *next = dep->next;
				if( next )
				{
					next->prev = NULL;
				}
				system->dependency_head = next;
				system->allocator->free( dep );
				dep = next;
				continue;
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
		LOG( "check dependency %s->%s \n" , dep->src , dep->dst );
		if( dep->src == module )
		{
			LOG( "found dependant module:%s\n" , dep->dst->name );
			releaseModule(  dep->dst , system );
			LOG( "searching to another dependant module for:%s\n" , module->name );
			goto dep_again;
		}
		dep = dep->next;
	}
}
void releaseModule( Module *module , ModuleSystem *system )
{
	int i;
	LOG( "disposing module %s\n" , module->name );
	if( module->prev )
	{
		module->prev->next = module->next;
	} else
	{
		system->modules_head = module->next;
	}
	if( module->next )
	{
		module->next->prev = module->prev;
	}
	LOG( "removed %s from list\n" , module->name );
	removeDependencies( system , module );
	LOG( "%s dependencies removed\n" , module->name );
	releaseDependantModules( system , module );
	LOG( "%s dependant modules removed\n" , module->name );
	/*for( i = 0; i < module->methods_count; i++ )
	{
		system->allocator->free( module->methods[ i ].arg_desc );
	}*/
	dlclose( module->lib_handle );
	LOG( "%s library handler disposed\n" , module->name );
	system->allocator->free( module->methods );
	system->allocator->free( module );
}
int strCmp( char const *a , char const *b )
{
	//printf( "comparing %c and %c\n" , *a , *b );
	while( *a != '\0' && *a++ == *b++ )
	{
		//printf( "comparing %c and %c\n" , *a , *b );
	}
	return *a == *b && *b == '\0';
}
Module *getModuleByName( char const *m_name , ModuleSystem *system )
{
	Module *m = system->modules_head;
	while( m )
	{
		if( strCmp( m->name , m_name ) )
		{
			return m;
		}
		m = m->next;
	}
	return NULL;
}
void releaseModuleByName( char const *m_name , ModuleSystem *system )
{
	Module *m = getModuleByName( m_name , system );
	if( m )
	{
		releaseModule( m , system );
	}
}
ModuleDependency *loadDependency( Module *dst , ModuleSystem *system )
{
	char full_name[ 0x100 ] = "modules/";
	int i;
	for( i = 0; i < strlen( dst->name ); i++ )
	{
		full_name[ i + 8 ] = dst->name[ i ];
	}

	int name_len = strlen( full_name );
	full_name[ name_len - 2 ] = 'd';
	full_name[ name_len - 1 ] = 'e';
	full_name[ name_len ] = 'p';
	full_name[ name_len + 1 ] = '\0';
	LOG( "trying to load dependency file:%s\n" , full_name );
	int dep_fd = open( full_name , O_RDONLY );

	int dep_counter = 0;
	if( dep_fd > 0 )
	{
		LOG( "success!\n" );
		while( 1 )
		{
			char c;
			size_t read_len;
			int name_len = 0;
			while( ( read_len = read( dep_fd , &c , 1 ) ) && c != '\n' )
			{
				full_name[ name_len++ ] = c;
			}
			if( name_len )
			{
				full_name[ name_len ] = '.';
				full_name[ name_len + 1 ] = 's';
				full_name[ name_len + 2 ] = 'o';
				full_name[ name_len + 3 ] = '\0';
				LOG( "trying to resolve dependency:%s\n" , full_name );
				Module *m = system->modules_head;
				while( m )
				{
					//LOG( "compare:%s and %s\n" , m->name , full_name );
					if( strCmp( m->name , full_name ) )
					{
						break;
					}
					m = m->next;
				}
				if( m && strCmp( m->name , full_name ) )
				{
					LOG( "module dependency:%s already loaded. using it\n" , full_name );
				} else
				{
					LOG( "module dependency:%s is not loaded. loadding it\n" , full_name );
					m = loadModule( full_name , system );
				}
				if( m )
				{
					ModuleDependency *dep_tail = system->dependency_head;
					while( dep_tail && dep_tail->next )
					{
						dep_tail = dep_tail->next;
					}
					dep_counter++;
					ModuleDependency *dep = system->allocator->alloc( sizeof( ModuleDependency ) );
					LOG( "inserting dependency:%s->%s\n" , full_name , dst->name );
					dep->next = NULL;
					dep->prev = dep_tail;
					dep->src = m;
					dep->dst = dst;
					if( dep_tail )
					{
						dep_tail->next = dep;
					} else
					{
						system->dependency_head = dep;
					}

					dep_tail = dep;
				}
			} else
			{
				break;
			}
		}
		close( dep_fd );
	}
}
Module *loadModule( char const *name , ModuleSystem *system )
{
	Module *out = ( Module* )system->allocator->alloc( sizeof( Module ) );
	strcopy( out->name , name );
	loadDependency( out , system );
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
		removeDependencies( system , out );
		system->allocator->free( out );
		return NULL;
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
		LOG( "could not pull unavoidable structs from module %s\n" , name );
		dlclose( lib );
		removeDependencies( system , out );
		system->allocator->free( out );

		return NULL;
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

	out->lib_handle = lib;

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
	return out;
	//symbol_table_size = ;

}
