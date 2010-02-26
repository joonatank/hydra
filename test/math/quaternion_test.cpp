#include "../../math/quaternion.hpp"

#include <assert.h>
#include <iostream>

int main( void )
{
	using vl::math::quaternion;
	using vl::math::vector;

	quaternion<double> q;
	quaternion<double> r(0.0, 0.0, 0.0, 1.0);
	assert( q.norm() == 1.0 && r.norm() == 1 );
	assert( q.unit() && r.unit() );
	assert( !q.reflection() && r.reflection() );
	assert( q.identity() && !r.identity() );
	assert( q != r );
	r = q;
	assert( q == r );

	std::cout << "Passed logical tests." << std::endl;
	
	// Test normalizing
	r.x = 1.0;	// r = (1, 1, 0, 0)
	assert( q != r && !r.unit() );
	r.normalize();	// r = (0.5, 0.5, 0, 0)
	std::cout << "r = " << r << " r.norm() = " << r.norm() << std::endl;
	assert( r.unit() );
	q = quaternion<double>( 1/std::sqrt(2), 1/std::sqrt(2), 0, 0 );
	assert( r == q );

	r.y = 1;
	r.z = 1;
	assert( !r.unit() );
	q =  normalize(r);
	assert( q.unit() && !r.unit() );

	std::cout << "Passed normalizing tests." << std::endl;

	r.reset();
	q.reset();
	assert( r == q );
	assert( r.identity() && r.unit() );

	// Test conjugate, both q.conj() and conj(q)
	// Conjugate will not modify identity quaternion
	q = conj(r);
	assert( q == r );
	q = quaternion<double>(1, 0, 0, 1);
	r = conj(q);
	assert( q != r );
	assert( r == quaternion<double>(1, 0, 0, -1) );
	q.conj();
	assert( q == r );
	r = quaternion<double>( 2, 2, 2, 2 ); 
	r.conj();
	assert( r == quaternion<double>( 2, -2, -2, -2 ) );
	assert( r == conj(conj(r)) );

	std::cout << "Passed conjugate tests. " << std::endl;

	// Test arithmetic assigment operators
	// operator+=(quat), operator-=(quat), operator*=(quat),
	// operator*=(scalar), operator/=(scalar)
	q.reset();
	r.reset();
	q += r;
	assert( q == quaternion<double>(2, 0, 0, 0) );
	r = quaternion<double>( 1, 2, 3, 1 );
	q = r;
	r += r;
	assert( r == quaternion<double>( 2, 4, 6, 2 ) );
	q *= 2;
	assert( q == r );
	q -= r;
	assert( q == quaternion<double>( 0, 0, 0, 0 ) );
	q -= r;
	assert( q== quaternion<double>( -2, -4, -6, -2 ) );
	q /= 2;
	assert( q == quaternion<double>( -1, -2, -3, -1 ) );

	// Multiplication
	r = quaternion<double>( -1, 0, 1, 0 );
	q *= r;
	std::cout << "q = " << q << std::endl;
	assert( q == quaternion<double>( 4, 3, 2, -1 ) );
	r *= r;
	assert( r == quaternion<double>( 0, 0, -2, 0 ) );
	r = quaternion<double>( -1, 0, 1, 0 );
	q = quaternion<double>( -1, -2, -3, -1 );
	r *= q;
	assert( r == quaternion<double>( 4, 1, 2, 3 ) );

	std::cout << "Passed arithmetic assignment tests." << std::endl;

	// Test arithmetic operators on const quats
	const double sq2 = std::sqrt(2);
	const quaternion<double> cq1(2, 0, 0, 4);
	const quaternion<double> cq2( 1/sq2, 0, 0, 1/sq2 );
	const quaternion<double> ident(1, 0, 0, 0);
	assert( !cq1.identity() && !cq2.identity() && ident.identity() );
	assert( !cq1.unit() && cq2.unit() && ident.unit() );
	
	// Addition
	assert( cq1+cq2 == quaternion<double>( 2+1/sq2, 0, 0, 4+1/sq2 ) );
	assert( cq1+ident == quaternion<double>( 2+1, 0, 0, 4 ) );
	assert( cq1+cq2+ident == quaternion<double>( 2+1/sq2+1, 0, 0, 4+1/sq2 ) );

	// Substraction
	assert( cq1-cq2 == quaternion<double>( 2-1/sq2, 0, 0, 4-1/sq2 ) );
	assert( cq1-ident == quaternion<double>( 2-1, 0, 0, 4 ) );
	assert( cq1-cq2-ident == quaternion<double>( 2-1/sq2-1, 0, 0, 4-1/sq2 ) );

	assert( cq1+cq2-ident == quaternion<double>( 2+1/sq2-1, 0, 0, 4+1/sq2 ) );
	assert( ident+cq1+cq2-cq1-cq2 == ident );

	// Scalar operators
	std::cout << "cq2*cq2 = " << cq2*cq2 << std::endl;
	assert( cq2*sq2 == quaternion<double>(1, 0, 0, 1) );
	assert( cq2*sq2*sq2 == quaternion<double>(sq2, 0, 0, sq2) );
	assert( cq1/sq2 == quaternion<double>( 2/sq2, 0, 0, 4/sq2 ) );
	assert( cq2*sq2*sq2/sq2/sq2 == cq2 );

	// Multiplication
	assert( cq2*cq2 == quaternion<double>( 0, 0, 0, 1 ) );
	assert( cq2*ident == cq2 );
	assert( ident*cq1 == cq1 );
	assert( cq1*(cq2*sq2) == quaternion<double>( -2, 0, 0, 6 ) );
	assert( (cq2*sq2)*cq1*(cq2*sq2) == quaternion<double>( -8, 0, 0, 4 ) );
	assert( (cq2*sq2)*cq1*(conj(cq2*sq2))
			== quaternion<double>( 4, 0, 0, 8 ) );
	assert( ident*cq1*(cq2*sq2)*ident == quaternion<double>( -2, 0, 0, 6 ) );

	std::cout << "Passed arithmetic tests." << std::endl;
	
	// Test conversions from angle-axis
	
	// Test conversions from euler angles
	
	// Test rotating and reflecting points
	// both quaternion and vector points

	return 0;
}
