#include "Complex.h"
#include <math.h>
#include <stdio.h>
#define SQRT sqrtf
#define ABS fabs
#define EPS 1.0e-37f
#define ATAN2 atan2f
#define test_def\
blah\
blah
Real c_mod2( Complex a )
{
	return a.r * a.r + a.i * a.i;
}
Real c_mod( Complex a )
{
	return SQRT( c_mod2( a ) );
}
Complex c_mulr( Complex a , Real r )
{
	return ( Complex ){ a.r * r , a.i * r };
}
Complex c_divr( Complex a , Real r )
{
	if( ABS( r ) < EPS )
	{
		return ( Complex ){ NAN , NAN };
	}
	return ( Complex ){ a.r / r , a.i / r };
}
Complex c_neg( Complex a )
{
	return c_mulr( a , ( Real )-1 );
}
Complex c_inv( Complex a )
{
	return c_divr( c_conjugate( a ) , c_mod2( a ) );
}
Complex c_conjugate( Complex a )
{
	return ( Complex ){ a.r , -a.i };
}
Complex c_add( Complex a , Complex b )
{
	return ( Complex ){ a.r + b.r , a.i + b.i };
}
Complex c_sub( Complex a , Complex b )
{
	return ( Complex ){ a.r - b.r , a.i - b.i };
}
Complex c_mul( Complex a , Complex b )
{
	return ( Complex ){ a.r * b.r - a.i * b.i , a.i * b.r + a.r * b.i };
}
Complex c_div( Complex a , Complex b )
{
	return c_mul( a , c_inv( b ) );
}
ComplexExp c_expComplex( Complex a )
{
	return ( ComplexExp ){ c_mod( a ) , ATAN2( a.i , a.r ) };
}
void c_printComplex( Complex a )
{
	printf( "{ r: %f i: %f }\n" , a.r , a.i );
}
void c_printComplexExponent( ComplexExp a )
{
	printf( "%f * e^i%f\n" , a.mag , a.angle );
}
