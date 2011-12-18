/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011/10 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-03
 *	@file math/math.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef HYDRA_MATH_MATH_HPP
#define HYDRA_MATH_MATH_HPP

// uint64_t uint32_t uint16_t and uint8_t
#include <stdint.h>

// Necessary for printing
#include <iostream>

// Necessary for the Wall configuration needed for calculating view and frustum matrices
#include "base/envsettings.hpp"

#include "types.hpp"
#include "transform.hpp"

#include <OGRE/OgreVector2.h>

namespace vl
{

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

inline bool equal(Ogre::Vector2 const &v1, Ogre::Vector2 const &v2 )
{
	return( equal(v1.x, v2.x) && equal(v1.y, v2.y) );
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

inline bool equal(vl::Transform const &t1, vl::Transform const &t2)
{
	return equal(t1.position, t2.position) && equal(t1.quaternion, t2.quaternion);
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
inline void clamp(T &x, T const &min, T const &max)
{
	x = (x < min) ? min : ((x > max) ? max : x);
}

template<>
inline void clamp(Ogre::Vector3 &v, Ogre::Vector3 const &min, Ogre::Vector3 const &max)
{
	clamp(v.x, min.x, max.x);
	clamp(v.y, min.y, max.y);	
	clamp(v.z, min.z, max.z);	
}

template<typename T>
inline T sign(T const &x)
{
	return (x > T(0)) ? T(1) : ((x < T(0)) ? T(-1) : T(0));
}

template<>
inline Ogre::Vector3 sign(Ogre::Vector3 const &x)
{
	return Ogre::Vector3(sign(x.x), sign(x.y), sign(x.z));
}

template<typename T>
inline T abs(T const &x)
{
	return (x > T(0)) ? x : -x;
}

template<typename T>
inline T const &max(T const &a, T const &b)
{
	return (a > b ? a : b);
}

template<typename T>
inline T const &min(T const &a, T const &b)
{
	return (a < b ? a : b);
}

void getEulerAngles( Ogre::Quaternion const &q, Ogre::Radian &x, Ogre::Radian &y, Ogre::Radian &z );

void fromEulerAngles( Ogre::Quaternion &q, Ogre::Radian const &rad_x,
						Ogre::Radian const &rad_y, Ogre::Radian const &rad_z );

/// @brief create a rotation that can be used to transform look to a point
/// @param target what we want to look at
/// @param current what are we looking at now
/// @param eye where are we looking from
/// @param up what our up vector is
/// @return quaternion that can be used to rotate from current orientation to target
/// @todo break this function to yaw fixed version that takes up axis as a param
/// and free yaw that does not.
Ogre::Quaternion
lookAt(Ogre::Vector3 const &target, Ogre::Vector3 const &current, 
	Ogre::Vector3 const &eye, Ogre::Vector3 const &up = Ogre::Vector3::UNIT_Y,
	bool yawFixed = true);



}	// namespace vl

#endif	// HYDRA_MATH_MATH_HPP
