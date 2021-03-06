/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-07
 *	@file math/types.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

/**	typedefs and includes for the math types
 *
 *	vector, quaternion, matrix, transform and scalar classes from another library
 *	also some predefined constants like PHI, epsilon and so on
 */

#ifndef HYDRA_MATH_TYPES_HPP
#define HYDRA_MATH_TYPES_HPP

// For MSV compilers to get M_PI
#define _USE_MATH_DEFINES
#include <math.h>

// Necessary for Math types
#include <OGRE/OgreQuaternion.h>
#include <OGRE/OgreVector2.h>
#include <OGRE/OgreVector3.h>
#include <OGRE/OgreMatrix4.h>
#include <OGRE/OgreMath.h>

namespace vl
{

typedef Ogre::Real scalar;

typedef Ogre::Vector3 Vector3;
typedef Ogre::Quaternion Quaternion;
typedef Ogre::Matrix4 Matrix4;

/// Constants
const scalar EPSILON = scalar(1e-6);

const scalar PHI = (1 + std::sqrt(5.0))/2;

// place holder struct for 2d integer vector
struct vec2i
{
	int x;
	int y;
	
	vec2i() {}
	vec2i(int x_, int y_) : x(x_), y(y_) {}
};

}	// namespace vl

#endif	// HYDRA_MATH_TYPES_HPP
