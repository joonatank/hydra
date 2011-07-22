/**	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-07
 *	@file math/types.hpp
 *
 *	typedefs and includes for the math types
 *
 *	vector, quaternion, matrix, transform and scalar classes from another library
 *	also some predefined constants like PHI, epsilon and so on
 */

#ifndef HYDRA_MATH_TYPES_HPP
#define HYDRA_MATH_TYPES_HPP

#include "transform.hpp"

// For MSV compilers to get M_PI
#define _USE_MATH_DEFINES
#include <math.h>

// Necessary for Math types
#include <OGRE/OgreQuaternion.h>
#include <OGRE/OgreVector3.h>
#include <OGRE/OgreMatrix4.h>

namespace vl
{

typedef Ogre::Real scalar;

typedef Ogre::Vector3 vec3;
typedef Ogre::Quaternion quaternion;
typedef Ogre::Matrix4 mat4;

/// Constants
const scalar epsilon = scalar(1e-6);

const scalar PHI = (1 + std::sqrt(5.0))/2;

}	// namespace vl

#endif	// HYDRA_MATH_TYPES_HPP
