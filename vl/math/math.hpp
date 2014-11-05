/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-03
 *	@file math/math.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

#ifndef HYDRA_MATH_MATH_HPP
#define HYDRA_MATH_MATH_HPP

// uint64_t uint32_t uint16_t and uint8_t
#include <stdint.h>

// Necessary for printing
#include <iostream>

// Using boost floating point for isnan is isinf
// because there is no standard way for this till c++0x
// all functions from here are in our namespace
#include <boost/math/special_functions/fpclassify.hpp>

#include "types.hpp"
#include "transform.hpp"

namespace vl
{

inline
bool is_power_of_two(uint64_t x)
{
    return (x & (x - 1)) == 0;
}

inline
uint64_t next_power_of_two(uint64_t x)
{
	x--;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	x |= x >> 32;
	x++;

	return x;
}

template<typename T>
inline bool equal(T const &a, T const &b, T const &epsilon)
{
	if( a-epsilon < b && a+epsilon > b )
	{ return true; }
	return false;
}

// Specializations
// Not template specialisation because the parameters don't match
// so just ordinary overloads
inline bool equal( Ogre::Vector3 const &v1, Ogre::Vector3 const &v2, vl::scalar const &epsilon = EPSILON)
{
	for( size_t i = 0; i < 3; ++i )
	{
		if( !equal(v1[i], v2[i], epsilon) )
		{ return false; }
	}
	return true;
}

inline bool equal(Ogre::Vector2 const &v1, Ogre::Vector2 const &v2 )
{
	return( equal(v1.x, v2.x, EPSILON) && equal(v1.y, v2.y, EPSILON) );
}

inline bool equal( Ogre::Quaternion const &q1, Ogre::Quaternion const &q2, vl::scalar const &epsilon = EPSILON)
{
	for( size_t i = 0; i < 4; ++i )
	{
		if( !equal(q1[i], q2[i], epsilon) )
		{ return false; }
	}
	return true;
}

inline bool equal(vl::Transform const &t1, vl::Transform const &t2, vl::scalar const &epsilon = EPSILON)
{
	return equal(t1.position, t2.position, epsilon) && equal(t1.quaternion, t2.quaternion, epsilon);
}

inline bool equal( Ogre::Matrix4 const &m1, Ogre::Matrix4 const &m2, vl::scalar const &epsilon = EPSILON)
{
	for( size_t i = 0; i < 4; ++i )
	{
		for( size_t j = 0; j < 4; ++j )
		{
			if(!equal( m1[i][j], m2[i][j], epsilon) )
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

/// @brief round a number
/// N is the number of decimals to which round the val
template<typename T, int N>
inline T round(T const &val)
{
	// @todo for the template to be useful we need to define pow and floor
	// as templates also.
	T h = std::pow(10, N);
	return( std::floor(val * h + 0.5) / h );
}

/// @brief Get the distance (angle between) two quaternions
/// @return angle between the quaternions in radians, always positive
inline vl::scalar
distance(Quaternion const &vecA, Quaternion const & vecB)
{
	Vector3 const &axis = Vector3::NEGATIVE_UNIT_Z;
	Vector3 snaDir = vecA * axis;
	Vector3 snbDir = vecB * axis; 
	return Ogre::Math::ACos(snaDir.dotProduct(snbDir)).valueRadians();
}

using boost::math::isfinite;
using boost::math::isinf;
using boost::math::isnan;
using boost::math::isnormal;

HYDRA_API void getEulerAngles( Ogre::Quaternion const &q, Ogre::Radian &x, Ogre::Radian &y, Ogre::Radian &z );

HYDRA_API void fromEulerAngles( Ogre::Quaternion &q, Ogre::Radian const &rad_x,
						Ogre::Radian const &rad_y, Ogre::Radian const &rad_z );

/// @brief create a rotation that can be used to transform look to a point
/// @param target what we want to look at
/// @param current what are we looking at now
/// @param eye where are we looking from
/// @param up what our up vector is
/// @return quaternion that can be used to rotate from current orientation to target
/// @todo break this function to yaw fixed version that takes up axis as a param
/// and free yaw that does not.
HYDRA_API Ogre::Quaternion
lookAt(Ogre::Vector3 const &target, Ogre::Vector3 const &current, 
	Ogre::Vector3 const &eye, Ogre::Vector3 const &up = Ogre::Vector3::UNIT_Y,
	bool yawFixed = true);

//HYDRA_API Ogre::Real
//getMatrix4Value(Ogre::Matrix4 &mat, size_t index);

}	// namespace vl

#endif	// HYDRA_MATH_MATH_HPP
