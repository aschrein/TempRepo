#define Real float
typedef struct
{
	Real r , i;
} Complex;
Real c_mod2( Complex );
Real c_mod( Complex );
Complex c_mulr( Complex , Real );
Complex c_divr( Complex , Real );
Complex c_neg( Complex );
Complex c_inv( Complex );
Complex c_conjugate( Complex );
Complex c_add( Complex , Complex );
Complex c_sub( Complex , Complex );
Complex c_mul( Complex , Complex );
Complex c_div( Complex , Complex );
void c_printComplex( Complex );
void c_printComplexExponent( ComplexExp );
typedef struct
{
	Real mag , angle;
} ComplexExp;
ComplexExp expComplex( Complex );
