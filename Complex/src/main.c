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

static inline int strCmp( char const *a , char const *b ){ while( *a != '\0' && *a++ == *b++ ); return *a == *b && *b == '\0'; }
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
				int name_len = strlen( dir->d_name );
				if( dir->d_name[ name_len - 1 ] != 'o' || dir->d_name[ name_len - 2 ] != 's' )
				{
					continue;
				}
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
int drawModule( void *window , ModuleView *mv )
{
	char const *loaded_msg = "(loaded)";
	char const *not_loaded_msg = "(not loaded)";
	char const *msg;
	if( mv->module )
	{
		msg = loaded_msg;
	} else
	{
		msg = not_loaded_msg;
	}
	ivec2 text_size = getTextSize( window , mv->name );
	ivec2 rect_size;
	{
		ivec2 msg_size = getTextSize( window , msg );
		rect_size.x = max( text_size.x , msg_size.x );
		rect_size.y = text_size.y + msg_size.y + 1;
	}
	PressMode pm;
	int ret = 0;
	if( ( pm = drawButton( window , mv->x - 1 , mv->y - text_size.y - 1 , rect_size.x + 2 , rect_size.y + 2 ) ) != PM_NONE )
	{
		//printf( "drag recieved!\n" );
		if( pm == PM_DRAG )
		{
			ivec2 dm = getDeltaMouse( window );
			mv->x += dm.x;
			mv->y += dm.y;
		} else if( pm == PM_LCLICK )
		{
			if( !mv->module )
			{
				ret = 1;
			}
		} else if( pm == PM_RCLICK )
		{
			if( mv->module )
			{
				ret = -1;
			}
		}
	}

	drawText( window , mv->x , mv->y , mv->name );
	drawText( window , mv->x , mv->y + text_size.y + 1 , msg );
	return ret;
}
typedef struct
{
	ModuleSystem *system;
	ModuleView *views;
} Context;
ModuleView *getViewByName( ModuleView *views , char const *mv_name )
{
	while( views )
	{
		if( strCmp( views->name , mv_name ) )
		{
			return views;
		}
		views = views->next;
	}
	return NULL;
}
int update_flag = 1;
Flow update( void *window , void *c )
{
	Context *ctx = ( Context* )c;

	ModuleView *mv = ctx->views;
	char const *text = "left click to load; right click to unload; drag to move block;";
	ivec2 text_size = getTextSize( window , text );
	drawText( window , 0 , text_size.y , text );
	drawLine( window , 0 , 12 , 512 , 12 );
	int y = 24;
	ModuleDependency *dep = ctx->system->dependency_head;
	while( dep )
	{
		printf( "rendering dependency:%s->%s\n" , dep->src , dep->dst );
		ModuleView *src_view = getViewByName( ctx->views , dep->src->name );
		ModuleView *dst_view = getViewByName( ctx->views , dep->dst->name );
		if( src_view && dst_view )
		{
			int midx = ( src_view->x + dst_view->x ) / 2;
			int midy = ( src_view->y + dst_view->y ) / 2;
			float dirx = ( src_view->x - dst_view->x );
			float diry = ( src_view->y - dst_view->y );
			float mod = sqrtf( dirx * dirx + diry * diry );
			dirx /= mod;
			diry /= mod;

			drawLine( window , src_view->x , src_view->y , dst_view->x , dst_view->y );
			drawLine( window , midx , midy , midx + ( int )( -diry * 10.0f - dirx * 10.0f + 0.5f ) ,
			midy + ( int )( dirx * 10.0f - diry * 10.0f + 0.5f ) );
			drawLine( window , midx , midy , midx + ( int )( diry * 10.0f - dirx * 10.0f + 0.5f ) ,
			midy + ( int )( -dirx * 10.0f - diry * 10.0f + 0.5f ) );
		}
		dep = dep->next;
	}
	int new_update = 0;
	while( mv )
	{
		if( update_flag )
		{
			mv->module = getModuleByName( mv->name , ctx->system );
		}
		printf( "rendering module:%s\n" , mv->name );
		int callback = drawModule( window , mv );
		if( callback == 1 )
		{
			//printf( "callback to load %s\n" , mv->name );
			loadModule( mv->name , ctx->system );
			new_update = 1;
		} else if( callback == -1 )
		{
			releaseModule( mv->module , ctx->system );
			new_update = 1;
		}
		mv = mv->next;
	}
	//update_flag = new_update;
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
	int y = 30;
	while( views )
	{
		views->x = 10;
		views->y = y;
		y += 30;
		views = views->next;
	}
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
