/**	@uathor Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2010-03
 *	@file conversion.hpp
 *
 *	Functions to convert math objects from one library to another.
 *	Ogre Math is our main math library.
 *	
 *	VMMLib <-> Ogre, added 2010-03
 *	VRPN -> Ogre, added 2010-12
 *	VMMLib removed 2010-02
 *
 *	TODO add conversion from bullet types
 */

#ifndef VL_MATH_CONVERSION_HPP
#define VL_MATH_CONVERSION_HPP

#include "math/math.hpp"

#include <OGRE/OgreVector3.h>
#include <OGRE/OgreQuaternion.h>
#include <OGRE/OgreMatrix4.h>
#include <OGRE/OgreColourValue.h>

// VRPN math types
#include <vrpn_Types.h>
#include <quat.h>

namespace vl
{

namespace math
{

// Convert from vrpn_float
inline
Ogre::Vector3 convert_vec( vrpn_float64 const *vec )
{
	scalar x = scalar(vec[Q_X]);
	scalar y = scalar(vec[Q_Y]);
	scalar z = scalar(vec[Q_Z]);
	return Ogre::Vector3( x, y, z );
}

inline
Ogre::Quaternion convert_quat( vrpn_float64 const *quat )
{
	scalar x = scalar(quat[Q_X]);
	scalar y = scalar(quat[Q_Y]);
	scalar z = scalar(quat[Q_Z]);
	scalar w = scalar(quat[Q_W]);
	return Ogre::Quaternion( w, x, y, z ); 
}

}	// namespace math

}	// namespace vl

#endif
