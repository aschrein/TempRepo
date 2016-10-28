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
void printComplex( Complex a )
{
	printf( "{ r: %f i: %f }\n" , a.r , a.i );
}
void printComplexExponent( ComplexExp a )
{
	printf( "%f * e^i%f\n" , a.mag , a.angle );
}
