//#include "include/Complex.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "Modules.h"
#include <Renderer.h>
int max( int a , int b )
{
	return a > b ? a : b;
}
static inline void strcopy( char *dst , char const *src ){ while( *dst++ = *src++ ); }
typedef struct ModuleView_t
{
	int x , y;
	char name[ 0x20 ];
	Module *module;
	struct ModuleView_t *next , *prev;
} ModuleView;
ModuleView *initViews()
{
	DIR *d;
	struct dirent *dir;
	d = opendir( "modules/" );
	ModuleView *head = NULL , *tail = NULL;
	if( d )
	{
		while( dir = readdir( d ) )
		{
			if( dir->d_type == DT_REG )
			{
				if( !head )
				{
					tail = head = ( ModuleView* )malloc( sizeof( ModuleView ) );
					memset( head , 0 , sizeof( ModuleView ) );
					strcopy( head->name , dir->d_name );
				} else
				{
					tail->next = ( ModuleView* )malloc( sizeof( ModuleView ) );
					memset( tail->next , 0 , sizeof( ModuleView ) );
					strcopy( tail->next->name , dir->d_name );
					tail->next->prev = tail;
					tail = tail->next;
				}
				//printf( "%s\n" , dir->d_name );
			}
		}
		closedir( dir );
	}
	return head;
}
void drawModule( void *window , ModuleView *mv )
{
	char const *loaded_msg = "(loaded)";
	char const *not_loaded_msg = "(not loaded)";
	char const *msg = mv->module ? loaded_msg : not_loaded_msg;
	ivec2 text_size = getTextSize( window , mv->name );
	ivec2 rect_size;
	{
		ivec2 msg_size = getTextSize( window , msg );
		rect_size.x = max( text_size.x , msg_size.x );
		rect_size.y = text_size.y + msg_size.y + 1;
	}
	drawButton( window , mv->x - 1 , mv->y - text_size.y - 1 , rect_size.x + 2 , rect_size.y + 2 );
	
	drawText( window , mv->x , mv->y , mv->name );
	drawText( window , mv->x , mv->y + text_size.y + 1 , msg );
	//y += rect_size.y + 2;
	/*int i = 0;
	for(; i < m->methods_count; i++ )
	{
		XDrawString( d , w , gc , 50 , y , m->methods[ i ].name , strlen( m->methods[ i ].name ) );
		y += 10;
	}*/
	//return y;
}
typedef struct
{
	ModuleSystem *system;
	ModuleView *views;
} Context;
Flow update( void *window , void *c )
{
	Context *ctx = ( Context* )c;
	
	ModuleView *mv = ctx->views;
	char const *text = "click to load; drag to move block;";
	ivec2 text_size = getTextSize( window , text );
	drawText( window , 0 , text_size.y , text );
	drawLine( window , 0 , 12 , 512 , 12 );
	int y = 24;
	while( mv )
	{
		drawModule( window , mv );
		mv = mv->next;
	}
	return CONTINUE;
}
int main( int argc , char **argv )
{
	Allocator allocator =
	{
		.free = free ,
		.alloc = malloc ,
		.realloc = realloc
	};
	ModuleSystem system =
	{
		.allocator = &allocator ,
		.modules_head = NULL ,
		.dependency_head = NULL
	};
	ModuleView *views = initViews();
	Context ctx = 
	{
		.system = &system ,
		.views = views
	};
	//listModules();
	loadModule( "c_inv.so" , &system );
	void *window = createWindow( 100 , 100 , 512 , 512 , update , &ctx );
	destroyWindow( window );
	//releaseModuleByName( "c_mod2.so" , &system );
	
	
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
