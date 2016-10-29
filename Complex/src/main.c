//#include "include/Complex.h"
#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Modules.h"

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
	Module *module0 = loadModule( "modules/c_conjugate.so" , allocator );
	Module *module1 = loadModule( "modules/c_mod2.so" , allocator );
	Module *module2 = loadModule( "modules/c_divr.so" , allocator );
	Module *module3 = loadModule( "modules/c_inv.so" , allocator );
	
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
