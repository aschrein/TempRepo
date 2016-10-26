#include "Complex.h"
#include <math.h>
#include <stdio.h>
#define SQRT sqrtf
#define ABS fabs
#define EPS 1.0e-37f
#define ATAN2 atan2f
Real mod2( Complex a )
{
	return a.r * a.r + a.i * a.i;
}
Real mod( Complex a )
{
	return SQRT( mod2( a ) );
}
Complex mulr( Complex a , Real r )
{
	return ( Complex ){ a.r * r , a.i * r };
}
Complex divr( Complex a , Real r )
{
	if( ABS( r ) < EPS )
	{
		return ( Complex ){ NAN , NAN };
	}
	return ( Complex ){ a.r / r , a.i / r };
}
Complex neg( Complex a )
{
	return mulr( a , ( Real )-1 );
}
Complex inv( Complex a )
{
	return divr( conjugate( a ) , mod2( a ) );
}
Complex conjugate( Complex a )
{
	return ( Complex ){ a.r , -a.i };
}
Complex add( Complex a , Complex b )
{
	return ( Complex ){ a.r + b.r , a.i + b.i };
}
Complex sub( Complex a , Complex b )
{
	return ( Complex ){ a.r - b.r , a.i - b.i };
}
Complex mul( Complex a , Complex b )
{
	return ( Complex ){ a.r * b.r - a.i * b.i , a.i * b.r + a.r * b.i };
}
Complex div( Complex a , Complex b )
{
	return mul( a , inv( b ) );
}
ComplexExp expComplex( Complex a )
{
	return ( ComplexExp ){ mod( a ) , ATAN2( a.i , a.r ) };
}
static void printComplex( Complex a )
{
	printf( "{ r: %f i: %f }\n" , a.r , a.i );
}
static void printComplexExponent( ComplexExp a )
{
	printf( "%f * e^i%f\n" , a.mag , a.angle );
}
int main( int argc , char **argv )
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
	printComplex( sub( ( Complex ){ 1.0f , 1.0f } , ( Complex ){ 1.0f , -1.0f } ) );//{ r: 0.000000 i: 2.000000 }
	printComplex( add( ( Complex ){ 1.0f , 1.0f } , ( Complex ){ 1.0f , -1.0f } ) );//{ r: 2.000000 i: 0.000000 }
	printf( "%f\n" , mod( ( Complex ){ 3.0f , 4.0f } ) );//5
	printComplex( neg( ( Complex ){ 3.0f , 1.0f } ) );//{ r: -3.000000 i: -1.000000 }
	printComplex( conjugate( ( Complex ){ 3.0f , 1.0f } ) );//{ r: 3.000000 i: -1.000000 }
	return 0;
}
