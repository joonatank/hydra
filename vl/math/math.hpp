/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-03
 *
 *	Base class for some math helper functions and typedefs for the math
 *	types we use.
 *	e.g. here you can change the vector, quaternion, matrix, scalar classes
 *	to use another library.
 */
#ifndef VL_MATH_MATH_HPP
#define VL_MATH_MATH_HPP

// For MSV compilers to get M_PI
#define _USE_MATH_DEFINES
#include <math.h>

#include <vmmlib/vector.hpp>
#include <vmmlib/math.hpp>
#include <vmmlib/quaternion.hpp>
#include <OGRE/OgreQuaternion.h>

namespace vl
{
	typedef vmml::quaterniond quaternion;
	typedef vmml::mat4d matrix;
	typedef vmml::vec3d vector;
	typedef double scalar;
	typedef vmml::vector<4,double> colour;
	typedef double angle;

	const scalar epsilon = 1e-6;

	inline bool equal( scalar const &a, scalar const &b )
	{
		if( a-epsilon < b && a+epsilon > b )
		{ return true; }
		return false;
	}

	inline bool equal( vector const &a, vector const &b )
	{
		if( (a.x()-epsilon < b.x() && a.x()+epsilon > b.x())
			&& (a.y()-epsilon < b.y() && a.y()+epsilon > b.y())
			&& (a.z()-epsilon < b.z() && a.z()+epsilon > b.z())
		  )
		{ return true; }
		return false;
	}

	inline bool equal( quaternion const &a, quaternion const &b )
	{
		if( (a.x()-epsilon < b.x() && a.x()+epsilon > b.x())
			&& (a.y()-epsilon < b.y() && a.y()+epsilon > b.y())
			&& (a.z()-epsilon < b.z() && a.z()+epsilon > b.z())
			&& (a.w()-epsilon < b.w() && a.w()+epsilon > b.w())
		  )
		{ return true; }
		return false;
	}

	inline
	void getEulerAngles( Ogre::Quaternion const &q, Ogre::Radian &x, Ogre::Radian &y, Ogre::Radian &z )
	{
		double test = q.x*q.y + q.z*q.w;
		if (test > 0.5-epsilon)
		{ // singularity at north pole
			y = 2 * atan2(q.x,q.w);
			z = M_PI/2;
			x = 0;
			return;
		}
		if (test < -0.5+epsilon)
		{ // singularity at south pole
			y = -2 * atan2(q.x,q.w);
			z = - M_PI/2;
			x = 0;
			return;
		}
		double sqx = q.x*q.x;
		double sqy = q.y*q.y;
		double sqz = q.z*q.z;
		// heading
		y = ::atan2( double(2*q.y*q.w-2*q.x*q.z), double(1 - 2*sqy - 2*sqz) );
		// attitude
		z = ::asin( double(2*test) );
		// bank
		x = ::atan2( double(2*q.x*q.w-2*q.y*q.z ), double( 1 - 2*sqx - 2*sqz) );
	}

	inline
	void fromEulerAngles( Ogre::Quaternion &q, Ogre::Radian const &rad_x,
						  Ogre::Radian const &rad_y, Ogre::Radian const &rad_z )
	{
		double heading = rad_y.valueRadians();
		double attitude = rad_z.valueRadians();
		double bank = rad_x.valueRadians();

		// Assuming the angles are in radians.
		double c1 = ::cos(heading/2);
		double s1 = ::sin(heading/2);
		double c2 = ::cos(attitude/2);
		double s2 = ::sin(attitude/2);
		double c3 = ::cos(bank/2);
		double s3 = ::sin(bank/2);
		double c1c2 = c1*c2;
		double s1s2 = s1*s2;
		q.w =c1c2*c3 - s1s2*s3;
		q.x =c1c2*s3 + s1s2*c3;
		q.y =s1*c2*c3 + c1*s2*s3;
		q.z =c1*s2*c3 - s1*c2*s3;
	}
}

#endif
