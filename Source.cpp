#include <cmath>
#include <cfloat>
#include <limits>
namespace SchreinerA
{
	static const double EPS = std::numeric_limits<double>::epsilon();
	//static const double MIN = std::numeric_limits<double>::min();
	//static const double DELTA = 1.0;
	struct vec3
	{
		double x , y , z;
		vec3 operator+( vec3 const &a ) const
		{
			return{ x + a.x , y + a.y , z + a.z };
		}
		vec3 operator-( vec3 const &a ) const
		{
			return{ x - a.x , y - a.y , z - a.z };
		}
		vec3 operator-() const
		{
			return{ -x , -y , -z };
		}
		vec3 operator*( double const &a ) const
		{
			return{ x * a , y * a , z * a };
		}
		double operator*( vec3 const &a ) const
		{
			return x * a.x + y * a.y + z * a.z;
		}
		vec3 operator/( double const &a ) const
		{
			return{ x / a , y / a , z / a };
		}
		vec3 norm() const
		{
			return this->operator/( mod() );
		}
		double mod2() const
		{
			return x * x + y * y + z * z;
		}
		double mod() const
		{
			return sqrt( mod2() );
		}
		vec3 operator^( vec3 const &b ) const
		{
			return
			{
				y * b.z - b.y * z ,
				b.x * z - x * b.z ,
				x * b.y - b.x * y
			};
		}
		bool isFinite() const
		{
			return isfinite( x ) && isfinite( y ) && isfinite( z ) &&
				!isnan( x ) && !isnan( y ) && !isnan( z )
				;
		}
		bool operator==( vec3 const &a ) const
		{
			return ( *this - a ).mod() / ( 1.0 + a.mod() ) / EPS < 10.0;
		}
	};
	struct plane
	{
		vec3 origin , normal;
		bool contains( vec3 const &p ) const
		{
			return abs( ( p - origin ) * normal ) / ( 1.0 + p.mod() ) / EPS < 10.0;
		}
		vec3 project( vec3 const &p ) const
		{
			return p - normal * ( normal * ( p - origin ) );
		}
		vec3 project( vec3 const &a , vec3 const &b ) const
		{
			vec3 v = ( b - a ).norm();
			double prj = -normal * ( a - origin );
			double dot = v * normal;
			if( abs( dot ) > 0.0 )
			{
				double t = prj / dot;
				if( t <= 0.0 )
				{
					return a;
				} else if( t >= ( a - b ).mod() )
				{
					return b;
				}
				return a + v * t;
			} else
			{
				return a;
			}
		}
	};
	struct triangle
	{
		vec3 a , b , c;
		double area() const
		{
			return ( ( b - a ) ^ ( c - a ) ).mod();
		}
		bool contains( vec3 const &p ) const
		{
			double ar0 = triangle{ a , b , p }.area() +
				triangle{ b , c , p }.area() +
				triangle{ c , a , p }.area();
			double ar1 = area();
			return
				abs( ar0 / ar1 - 1.0 ) / EPS < 100.0;
		}
	};
	struct edge
	{
		vec3 origin , end;
		bool intersects( vec3 const &a ) const
		{
			if( a == origin || a == end )
			{
				return true;
			}
			if( origin == end )
			{
				return false;
			}
			return abs(
				( ( a - end ).mod() +
				( a - origin ).mod() ) /
				( origin - end ).mod() - 1.0 ) / EPS < 10.0;
		}
		bool intersects( vec3 const &a , vec3 const &b ) const//no degenerate cases
		{
			if(
				a == origin ||
				a == end ||
				b == origin ||
				b == end
				)
			{
				return true;
			}
			plane p{ a , ( ( b - a ) ^ ( origin - a ) ).norm() };
			if( p.contains( end ) )
			{
				vec3 v0 = ( end - origin ).norm();
				vec3 v1 = ( b - a ).norm();
				double det = v0.x * v1.y - v1.x * v0.y;
				vec3 dp = a - origin;
				double mod0 = ( end - origin ).mod();
				double mod1 = ( a - b ).mod();
				if( abs( det ) > EPS )
				{
					double t0 = ( v1.y * dp.x - v1.x * dp.y ) / det;
					double t1 = -( -v0.y * dp.x + v0.x * dp.y ) / det;
					return
						t0 >= -EPS * mod0 && t0 <= mod0 * ( 1.0 + EPS ) &&
						t1 >= -EPS * mod1 && t1 <= mod1 * ( 1.0 + EPS )
						;
				} else
				{
					det = v0.y * v1.z - v0.z * v1.y;
					double t0 = ( v1.z * dp.y - v1.y * dp.z ) / det;
					double t1 = -( -v0.z * dp.y + v0.y * dp.z ) / det;
					return
						t0 >= -EPS * mod0 && t0 <= mod0 * ( 1.0 + EPS ) &&
						t1 >= -EPS * mod1 && t1 <= mod1 * ( 1.0 + EPS )
						;
				}
			} else
			{
				return false;
			}
		}
		bool intersects( vec3 const &a , vec3 const &b , vec3 const &c ) const
		{
			if(
				!end.isFinite() ||
				!origin.isFinite() ||
				!a.isFinite() ||
				!b.isFinite() ||
				!c.isFinite()
				)
			{
				return false;
			}
			if( triangle{ a , b , c }.contains( end ) || triangle{ a , b , c }.contains( origin ) )
			{
				return true;
			}
			//degenerate cases
			if( origin == end )//o->e is point
			{
				if( edge{ a , b }.intersects( c ) )//triangle is line a->b
				{
					if( b == a )//a->b is point
					{
						return a == origin;
					} else
					{
						return edge{ a , b }.intersects( origin );
					}
				} else if( edge{ a , c }.intersects( b ) )//triangle is line a->c
				{
					return edge{ a , c }.intersects( origin );
				} else if( edge{ b , c }.intersects( a ) )//triangle is line b->c
				{
					return edge{ b , c }.intersects( origin );
				} else
				{
					return triangle{ a , b , c }.contains( origin );
				}
			}
			if( edge{ a , b }.intersects( c ) )//triangle is line a->b
			{
				if( b == a )//a->b is point
				{
					return intersects( a );
				} else
				{
					return edge{ a , b }.intersects( origin , end );
				}
			} else if( edge{ a , c }.intersects( b ) )//triangle is line a->c
			{
				return edge{ a , c }.intersects( origin , end );
			} else if( edge{ b , c }.intersects( a ) )//triangle is line b->c
			{
				return edge{ b , c }.intersects( origin , end );
			} else
			{
				plane p = { a , ( ( b - a ) ^ ( c - a ) ).norm() };
				if( abs( p.normal * ( origin - end ).norm() ) < EPS )
				{
					return intersects( a , b ) ||
						intersects( b , c ) ||
						intersects( c , a ) ||
						triangle{ a , b , c }.contains( origin ) ||
						triangle{ a , b , c }.contains( end );
				}
				vec3 proj = p.project( origin , end );
				return triangle{ a , b , c }.contains( proj );
			}
		}
	};
	bool intersects( double *a , double *b )
	{
		vec3 *arr0 = ( vec3* )a;
		vec3 *arr1 = ( vec3* )b;
		for( int i = 0; i < 3; i++ )
		{
			if( !arr0[ i ].isFinite() )
			{
				return false;
			}
			if( !arr1[ i ].isFinite() )
			{
				return false;
			}
		}
		for( int i = 0; i < 3; i++ )
		{
			if( edge{ arr0[ i ] , arr0[ ( i + 1 ) % 3 ] }.intersects( arr1[ 0 ] , arr1[ 1 ] , arr1[ 2 ] ) )
			{
				return true;
			}
		}
		for( int i = 0; i < 3; i++ )
		{
			if( edge{ arr1[ i ] , arr1[ ( i + 1 ) % 3 ] }.intersects( arr0[ 0 ] , arr0[ 1 ] , arr0[ 2 ] ) )
			{
				return true;
			}
		}
		return false;
	}
}