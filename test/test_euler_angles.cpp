
#ifdef VL_UNIX
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MODULE euler_angles

#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <OGRE/OgreVector3.h>

// Tested header
#include "math/math.hpp"

const Ogre::Real tolerance = 1e-3;

using namespace Ogre;

BOOST_AUTO_TEST_CASE( convert_quaternion )
{
	// Test x axis rotation
	for( size_t i = 0; i < 90; ++i )
	{
		Degree deg(i);
		Quaternion q( Radian(deg), Vector3::UNIT_X );
		Radian x, y, z;
		vl::getEulerAngles( q, x, y, z );
		BOOST_CHECK_CLOSE( deg.valueRadians(), x.valueRadians(), tolerance );
		BOOST_CHECK_SMALL( y.valueRadians(), tolerance );
		BOOST_CHECK_SMALL( z.valueRadians(), tolerance );
	}

	// Test y axis rotation
	for( size_t i = 0; i < 90; ++i )
	{
		Degree deg(i);
		Quaternion q( Radian(deg), Vector3::UNIT_Y );
		Radian x, y, z;
		vl::getEulerAngles( q, x, y, z );
		BOOST_CHECK_SMALL( x.valueRadians(), tolerance );
		BOOST_CHECK_CLOSE( deg.valueRadians(), y.valueRadians(), tolerance );
		BOOST_CHECK_SMALL( z.valueRadians(), tolerance );
	}

	// Test z axis rotation
	for( size_t i = 0; i < 90; ++i )
	{
		Degree deg(i);
		Quaternion q( Radian(deg), Vector3::UNIT_Z );
		Radian x, y, z;
		vl::getEulerAngles( q, x, y, z );
		BOOST_CHECK_SMALL( x.valueRadians(), tolerance );
		BOOST_CHECK_SMALL( y.valueRadians(), tolerance );
		BOOST_CHECK_CLOSE( deg.valueRadians(), z.valueRadians(), tolerance );
	}
}

// FIXME this doesn't pass
// probably because the you can get the same rotation with different angles
// so our comparison is naive at best.
BOOST_AUTO_TEST_CASE( convert_compined_quaternion )
{
	// compined rotations
	for( size_t i = 0; i < 90; ++i )
	{
		Degree deg_x(i);
		Degree deg_y(-i);
		Degree deg_z(45-i);
		Quaternion q_x( Radian(deg_x), Vector3::UNIT_X );
		Quaternion q_y( Radian(deg_y), Vector3::UNIT_Y );
		Quaternion q_z( Radian(deg_z), Vector3::UNIT_Z );
		Quaternion q = q_y * q_z *q_x;

		Radian x, y, z;
		vl::getEulerAngles( q, x, y, z );

		// Check the values
		Ogre::Real rad_x = deg_x.valueRadians();
		Ogre::Real rad_y = deg_y.valueRadians();
		Ogre::Real rad_z = deg_z.valueRadians();
		if( abs(rad_x) < vl::epsilon )
			BOOST_CHECK_SMALL(  x.valueRadians(), tolerance );
		else
			BOOST_CHECK_CLOSE( rad_x, x.valueRadians(), tolerance );

		if( abs(rad_y) < vl::epsilon )
			BOOST_CHECK_SMALL(  y.valueRadians(), tolerance );
		else
			BOOST_CHECK_CLOSE( rad_y, y.valueRadians(), tolerance );

		if( abs(rad_z) < vl::epsilon )
			BOOST_CHECK_SMALL(  z.valueRadians(), tolerance );
		else
			BOOST_CHECK_CLOSE( rad_z, z.valueRadians(), tolerance );
	}
}

