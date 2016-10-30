#ifndef MODULES_H
#define MODULES_H
#include <stdint.h>
#include <stdlib.h>
#include <Allocator.h>
/*typedef struct
{
	char type[ 0x10 ];
	char name[ 0x10 ];
} ArgDesc;*/
typedef struct
{
	//ArgDesc *arg_desc;
	char name[ 0x10 ];
	void *ptr;
	//uint32_t arg_count;
} Method;
typedef struct Module_t
{
	char name[ 0x10 ];
	Method *methods;
	void *lib_handle;
	struct Module_t *next , *prev;
	uint32_t methods_count;
} Module;
typedef struct ModuleDependency_t
{
	Module *src;
	Module *dst;
	struct ModuleDependency_t *next , *prev;
} ModuleDependency;
typedef struct
{
	Allocator *allocator;
	Module *modules_head;
	ModuleDependency *dependency_head;
} ModuleSystem;
void releaseModule( ModuleSystem * ,  Module * );
static char const * const MODULES_DIR = "modules";
int loadModule( char const * , ModuleSystem * );
#endif
