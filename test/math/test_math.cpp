#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE vl_math

#include <boost/test/unit_test.hpp>

#include "math/math.hpp"

BOOST_AUTO_TEST_CASE( vector_test )
{
	vl::vector v(0.0,0.0,0.0);
	v += vl::vector(0.0, 1.0, 2.0);

	BOOST_CHECK( vl::equal(v, vl::vector(0.0, 1.0, 2.0) ) );
	BOOST_CHECK_EQUAL( v, vl::vector(0.0, 1.0, 2.0) );
}

BOOST_AUTO_TEST_CASE( quaternion_test )
{
	vl::quaternion q(0, 0, 0, 1);
	q *= vl::quaternion(0.7071, 0, 0, 0.7071);

	BOOST_CHECK( vl::equal(q, vl::quaternion(0.7071, 0, 0, 0.7071) ) );
	BOOST_CHECK_EQUAL( q, vl::quaternion(0.7071, 0, 0, 0.7071) );

	vl::scalar half_a = (M_PI/2)/2;
	q = vl::quaternion( std::sin(half_a), 0, 0, std::cos(half_a) );
	BOOST_CHECK_EQUAL( q.w(), std::cos(half_a) );
	BOOST_CHECK_EQUAL( q.x(), std::sin(half_a) );
	BOOST_CHECK_EQUAL( q.y(), 0 );
	BOOST_CHECK_EQUAL( q.y(), 0 );
}

BOOST_AUTO_TEST_CASE( matrix_test )
{
	vl::matrix m;
}

