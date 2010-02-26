/*	Joonatan Kuosa
 *	2010-02
 *
 *	Unit tests for the vector class
 */

#include "../../math/vector.hpp"

#include <iostream>
#include <assert.h>

int main( void )
{
	std::cout << "Vector test. " << std::endl;

	vl::math::vector<3, double> a;
	a[0] = 1;
	a[1] = 2;

	double array[3] = {1, 2, 0};
	vl::math::vec3d b(array);

	//std::cout << "Print " << a[0] << " " << a[1];
	std::cout << "Testing logical operators" << std::endl;
	std::cout << "a = " << a << std::endl;
	std::cout << "b = " << a << std::endl;

	assert( a == b );

	b[0] += 5;
	b[1] += 5;
	b[2] += 5;

	assert( a != b );

	a += 5;
	assert( a == b );

	std::cout << "Passed logical operator tests" << std::endl;

	vl::math::vec3d c( a -b );
	assert( c.zero() );

	c[0] = 3;
	std::cout << c << " : norm = " << c.norm() << std::endl;
	assert( c.norm() == 3);

	c[0] = 1;
	assert( c.unit() );

	c[1] = 1;
	assert( !c.unit() );

	c.normalize();
	std::cout << "c = " << c << " : norm " << c.norm() << std::endl;
	assert( c.unit() );

	// Test operators
	// d = (1, 2, 0)
	std::cout << "Testing arithmetic operators." << std::endl;
	vl::math::vec3d d( array );
	c = d;
	assert( c == d );
	
	a[0] = 0;
	a[1] = 1;
	a[2] = 0;

	d += a;
	assert( d == c + a );
	assert( d[0] == 1+0 && d[1] == 2+1 && d[2] == 0+0 );
	assert( d-a == c );
	c *= 5;
	assert( c[0] == 5 && c[1] == 10 && c[2] == 0 );
	c /= 5;
	assert( c[0] == 1 && c[1] == 2 && c[2] == 0 );

	std::cout << "Passed arithmetic operator tests." << std::endl;

	// Test dot and cross products
	std::cout << "Test dot and cross products : c = " << c << std::endl;
	assert( c.dot(c) ==  ( 1 + 2*2 + 0 ) );
	assert( std::sqrt( vl::math::dot( c, c ) ) == c.norm() );
	assert( vl::math::dot( a, c ) == 2 );

	d[0] = 0.0;
	d[1] = 5.0;
	d[2] = 3.0;
	assert( vl::math::dot( c, d ) == 0+10+0 );

	assert( d[0] == 0 && d[1] == 5 && d[2] == 3 );
	assert( c[0] == 1 && c[1] == 2 && c[2] == 0 );
	a[0] = -6;
	a[1] = 3;
	a[2] = -5;

	b = vl::math::cross( d, c);
	std::cout << " c x d == " << b << std::endl;
	assert( b == a );
	b = vl::math::cross( c, d);
	assert( b == a*(-1.0) );
	std::cout << "Passed dot and cross product tests." << std::endl;

	return 0;
}

