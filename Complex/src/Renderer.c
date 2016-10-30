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
	int rmb , lmb , l_lmb , l_rmb , lclck , rclck , possible_click;
} HWindow;
void *createWindow( int x , int y , int width , int height , Flow ( *update )( void * , void * ) , void *ctx )
{
	HWindow *out = ( HWindow* )malloc( sizeof( HWindow ) );
	memset( out , 0 , sizeof( HWindow ) );
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
	XSelectInput( out->d , out->w , ExposureMask | KeyPressMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask );
	XMapWindow( out->d , out->w );
	//XGCValues gcval;
	out->gc = DefaultGC( out->d , out->s );
	while( 1 )
	{
		XEvent e;
		XNextEvent( out->d , &e );
		if( e.type == Expose )
		{
			XClearWindow( out->d , out->w );
			//printf( "button press:%i\n" , out->lmb );
			update( out , ctx );
			
			out->l_lmb = out->lmb;
			out->l_rmb = out->rmb;
			out->l_mx = out->mx;
			out->l_my = out->my;
			out->lclck = 0;
			out->rclck = 0;
			XEvent ex;
			ex.type = Expose;
			ex.xexpose.window = out->w;
			XSendEvent( out->d , out->w , 0 , ExposureMask , &ex );
			XFlush( out->d );
			usleep( 1000 );
		} else if( e.type == KeyPress )
		{
		} else if( e.type == ButtonPress )
		{
			out->possible_click = 1;
			if( e.xbutton.button == Button1 )
			{
				out->lmb = 1;
			} else if( e.xbutton.button == Button3 )
			{
				out->rmb = 1;
			}
			
			//printf( "button press\n" );
		} else if( e.type == ButtonRelease )
		{
			if( e.xbutton.button == Button1 )
			{
				out->lclck = out->possible_click;
				if( out->lclck )
				{
					puts( "lclick!\n" );
				}
				out->lmb = 0;
			} else if( e.xbutton.button == Button3 )
			{
				out->rclck = out->possible_click;
				if( out->rclck )
				{
					puts( "rclick!\n" );
				}
				out->rmb = 0;
			}
			
			out->possible_click = 0;
			//printf( "button release\n" );
		} else if( e.type == MotionNotify )
		{
			out->possible_click = 0;
			out->mx = e.xmotion.x;
			out->my = e.xmotion.y;
			//printf( "pointer motion x:%i , y:%i\n" , e.xmotion.x , e.xmotion.y );
		} else
		{
			
		}
			
	}
	return out;
}
void destroyWindow( void *window )
{
	HWindow *w = ( HWindow* )window;
	XCloseDisplay( w );
	free( w );
}
ivec2 getDeltaMouse( void *window )
{
	HWindow *w = ( HWindow* )window;
	return ( ivec2 ){ .x = w->mx - w->l_mx , .y = w->my - w->l_my };
}
PressMode drawButton( void *window , int x , int y , int width , int height )
{
	HWindow *w = ( HWindow* )window;
	XSetForeground( w->d , w->gc , WhitePixel( w->d , w->s ) );
	XFillRectangle( w->d , w->w , w->gc , x , y , width , height );
	XSetForeground( w->d , w->gc , BlackPixel( w->d , w->s ) );
	XDrawRectangle( w->d , w->w , w->gc , x , y , width , height );
	int inside =
	w->mx > x && w->mx < x + width && w->my > y && w->my < y + height
	|| w->l_mx > x && w->l_mx < x + width && w->l_my > y && w->l_my < y + height
	;
	if( inside )
	{
		ivec2 dm = getDeltaMouse( w );
		if( w->lclck )
		{
			return PM_LCLICK;
		}
		if( w->rclck )
		{
			return PM_RCLICK;
		}
		if( w->lmb && ( dm.x || dm.y ) )
		{
			//puts( "drag sent!\n" );
			return PM_DRAG;
		}
	}
	return PM_NONE;
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
