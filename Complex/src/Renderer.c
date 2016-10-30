#include <Renderer.h>
#include <X11/Xlib.h>
typedef struct
{
	Display *d;
	int s;
	Window w;
	GC gc;
	int x , y , width , height;
	int mx , my , l_mx , l_my;
	int rmb , lmb , l_lmb , l_rmb;
} HWindow;
void *createWindow( int x , int y , int width , int height , Flow ( *update )( void * , void * ) , void *ctx )
{
	HWindow *out = ( Window* )malloc( sizeof( HWindow ) );
	out->d = XOpenDisplay( NULL );
	out->x = x;
	out->y = y;
	out->width = width;
	out->height = height;
	if( out->d == NULL )
	{
		printf( "Cannot open display\n" );
		free( out );
		return NULL;
	}
	
	out->s = DefaultScreen( out->d );
	out->w = XCreateSimpleWindow( out->d , RootWindow( out->d , out->s ) ,
	x , y , width , height , 1 , BlackPixel( out->d , out->s ) , WhitePixel( out->d , out->s ) );
	XSelectInput( out->d , out->w , ExposureMask | KeyPressMask );
	XMapWindow( out->d , out->w );
	XGCValues gcval;
	out->gc = DefaultGC( out->d , out->s );
	while( 1 )
	{
		XEvent e;
		XNextEvent( out->d , &e );
		if( e.type == Expose )
		{
			update( out , ctx );
			
			/*int y = 20;
			XDrawString( d , w , gc , 10 , 10 , "Loaded Modules:" , strlen( "Loaded Modules:" ) );
			XDrawLine( d , w , gc , 0 , 10 , 512 , 10 );
			Module *m = system.modules_head;
			while( m )
			{
				y = drawModule( m , 10 , y );
				m = m->next;
			}*/
			/*ModuleDependency *dep = system.dependency_head;
			while( dep )
			{
				printf( "%s->%s\n" , dep->src , dep->dst );
				dep = dep->next;
			}*/
			//XFillRectangle( d , w , DefaultGC( d , s ) , 20 , 20 , 10 , 10 );
			//XDrawString( d , w , DefaultGC( d , s ) , 10 , 50 , "hello!" , 6 );
			XFlush( out->d );
		}
		if (e.type == KeyPress)
		break;
	}
	return out;
}
void destroyWindow( void *window )
{
	HWindow *w = ( HWindow* )window;
	XCloseDisplay( w );
	free( w );
}
ivec2 getDeltaMouse( void *w )
{
	
}
PressMode drawButton( void *window , int x , int y , int width , int height )
{
	HWindow *w = ( HWindow* )window;
	XDrawRectangle( w->d , w->w , w->gc , x , y , width , height );
}
void drawText( void *window , int x , int y , char const *text )
{
	HWindow *w = ( HWindow* )window;
	XDrawString( w->d , w->w , w->gc , x , y , text , strlen( text ) );
}
ivec2 getTextSize( void *w , char const *text )
{
	return ( ivec2 ){ .x = strlen( text ) * 6 , .y = 10 };
}
void drawLine( void *window , int x0 , int y0 , int x1 , int y1 )
{
	HWindow *w = ( HWindow* )window;
	XDrawLine( w->d , w->w , w->gc , x0 , y0 , x1 , y1 );
}
