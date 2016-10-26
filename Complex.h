#define Real float
typedef struct
{
	Real r , i;
} Complex;
Real mod2( Complex );
Real mod( Complex );
Complex mulr( Complex , Real );
Complex divr( Complex , Real );
Complex neg( Complex );
Complex inv( Complex );
Complex conjugate( Complex );
Complex add( Complex , Complex );
Complex sub( Complex , Complex );
Complex mul( Complex , Complex );
Complex div( Complex , Complex );
typedef struct
{
	Real mag , angle;
} ComplexExp;
ComplexExp expComplex( Complex );
