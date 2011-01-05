
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

// Necessary for Math types
#include <OGRE/OgreQuaternion.h>
#include <OGRE/OgreVector3.h>
#include <OGRE/OgreMatrix4.h>

// Necessary for printing
#include <iostream>

namespace vl
{
	typedef Ogre::Real scalar;
	typedef Ogre::Real angle;

	const scalar epsilon = scalar(1e-6);

	inline bool equal( scalar const &a, scalar const &b )
	{
		if( a-epsilon < b && a+epsilon > b )
		{ return true; }
		return false;
	}

	inline bool equal( Ogre::Vector3 const &v1, Ogre::Vector3 const &v2 )
	{
		for( size_t i = 0; i < 3; ++i )
		{
			if( !equal( v1[i], v2[i] ) )
			{ return false; }
		}
		return true;
	}

	inline bool equal( Ogre::Quaternion const &q1, Ogre::Quaternion const &q2 )
	{
		for( size_t i = 0; i < 4; ++i )
		{
			if( !equal( q1[i], q2[i] ) )
			{ return false; }
		}
		return true;
	}

	inline bool equal( Ogre::Matrix4 const &m1, Ogre::Matrix4 const &m2 )
	{
		for( size_t i = 0; i < 4; ++i )
		{
			for( size_t j = 0; j < 4; ++j )
			{
				if( !equal( m1[i][j], m2[i][j] ) )
				{ return false; }
			}
		}
		return true;
	}

	struct Transform
	{
		Transform( Ogre::Vector3 const &pos = Ogre::Vector3::ZERO,
					Ogre::Quaternion const &rot = Ogre::Quaternion::IDENTITY )
			: position( pos ), quaternion( rot )
		{}

		Ogre::Vector3 position;
		Ogre::Quaternion quaternion;
	};

	inline std::ostream &operator<<( std::ostream &os, Transform const &d )
	{
		os << "Position = " << d.position << " : Orientation = " << d.quaternion;

		return os;
	}

	inline
	void getEulerAngles( Ogre::Quaternion const &q, Ogre::Radian &x, Ogre::Radian &y, Ogre::Radian &z )
	{
		scalar test = q.x*q.y + q.z*q.w;
		if (test > 0.5-epsilon)
		{ // singularity at north pole
			y = 2 * atan2(q.x,q.w);
			z = scalar(M_PI/2);
			x = 0;
			return;
		}
		if (test < -0.5+epsilon)
		{ // singularity at south pole
			y = -2 * atan2(q.x,q.w);
			z = scalar(- M_PI/2);
			x = 0;
			return;
		}

		scalar sqx = q.x*q.x;
		scalar sqy = q.y*q.y;
		scalar sqz = q.z*q.z;
		// heading
		y = ::atan2( scalar(2*q.y*q.w-2*q.x*q.z), scalar(1 - 2*sqy - 2*sqz) );
		// attitude
		z = ::asin( scalar(2*test) );
		// bank
		x = ::atan2( scalar(2*q.x*q.w-2*q.y*q.z ), scalar( 1 - 2*sqx - 2*sqz) );
	}

	inline
	void fromEulerAngles( Ogre::Quaternion &q, Ogre::Radian const &rad_x,
						  Ogre::Radian const &rad_y, Ogre::Radian const &rad_z )
	{
		scalar heading = rad_y.valueRadians();
		scalar attitude = rad_z.valueRadians();
		scalar bank = rad_x.valueRadians();

		// Assuming the angles are in radians.
		scalar c1 = ::cos(heading/2);
		scalar s1 = ::sin(heading/2);
		scalar c2 = ::cos(attitude/2);
		scalar s2 = ::sin(attitude/2);
		scalar c3 = ::cos(bank/2);
		scalar s3 = ::sin(bank/2);
		scalar c1c2 = c1*c2;
		scalar s1s2 = s1*s2;
		q.w =c1c2*c3 - s1s2*s3;
		q.x =c1c2*s3 + s1s2*c3;
		q.y =s1*c2*c3 + c1*s2*s3;
		q.z =c1*s2*c3 - s1*c2*s3;
	}
}

#endif
