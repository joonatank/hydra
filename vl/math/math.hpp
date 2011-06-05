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

const scalar PHI = (1 + std::sqrt(5.0))/2;

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

template<typename T>
inline T sign(T const &x)
{
	return (x > 0) ? 1 : ((x < 0) ? -1 : 0);
}

template<>
inline Ogre::Vector3 sign(Ogre::Vector3 const &x)
{
	return Ogre::Vector3(sign(x.x), sign(x.y), sign(x.z));
}

void getEulerAngles( Ogre::Quaternion const &q, Ogre::Radian &x, Ogre::Radian &y, Ogre::Radian &z );

void fromEulerAngles( Ogre::Quaternion &q, Ogre::Radian const &rad_x,
						Ogre::Radian const &rad_y, Ogre::Radian const &rad_z );

/// @brief calculate projetion matrix using VR parameters
/// @param near_plane the near clipping plane distance
/// @param far_plane the far clipping plane distance
/// @param wall the projection wall/screen used for this frustum
/// @param head the head transformation used for this projection
/// @return OpenGL projection matrix
/// @todo add asymmetric stereo frustum support (needs head rotation)
Ogre::Matrix4
calculate_projection_matrix(Ogre::Real near_plane, Ogre::Real far_plane, 
							vl::EnvSettings::Wall const &wall,
							Ogre::Vector3 const &head);

/// @brief calculate the orientation from eye space to wall
Ogre::Quaternion orientation_to_wall(vl::EnvSettings::Wall const &wall);

}	// namespace vl

#endif
