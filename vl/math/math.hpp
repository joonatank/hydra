/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2010-03
 *	@file math.hpp
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

// Necessary for the Wall configuration needed for calculating view and frustum matrices
#include "base/envsettings.hpp"

namespace vl
{

typedef Ogre::Real scalar;

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

	vl::Transform &operator*=( Ogre::Matrix4 const &m )
	{
		Ogre::Matrix4 m2( quaternion );
		m2.setTrans( position );
		Ogre::Matrix4 res = m2  * m;
		position = res.getTrans();
		quaternion = res.extractQuaternion();
		quaternion.normalise();
		return *this;
	}

	Ogre::Vector3 position;
	Ogre::Quaternion quaternion;
};

std::ostream &
operator<<( std::ostream &os, Transform const &d );

vl::Transform 
operator*( vl::Transform const &t, Ogre::Matrix4 const &m );

vl::Transform 
operator*( Ogre::Matrix4 const &m, vl::Transform const &t );

void getEulerAngles( Ogre::Quaternion const &q, Ogre::Radian &x, Ogre::Radian &y, Ogre::Radian &z );

void fromEulerAngles( Ogre::Quaternion &q, Ogre::Radian const &rad_x,
						Ogre::Radian const &rad_y, Ogre::Radian const &rad_z );

Ogre::Matrix4 
calculate_projection_matrix(Ogre::Real c_near, Ogre::Real c_far, 
							vl::EnvSettings::Wall const &wall);

Ogre::Matrix4 
calculate_view_matrix(Ogre::Vector3 const &camera_pos, 
					  Ogre::Quaternion const &camera_orient,
					  vl::EnvSettings::Wall const &wall,
					  Ogre::Matrix4 const &head,
					  Ogre::Vector3 const &eye = Ogre::Vector3::ZERO);


}	// namespace vl

#endif
