/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2010-05
 *	@file transform.hpp
 *
 */

#ifndef HYDRA_MATH_TRANSFORM_HPP
#define HYDRA_MATH_TRANSFORM_HPP

// Necessary for Math types
#include <OGRE/OgreQuaternion.h>
#include <OGRE/OgreVector3.h>
#include <OGRE/OgreMatrix4.h>

namespace vl
{

struct Transform
{
	Transform( Ogre::Vector3 const &pos = Ogre::Vector3::ZERO,
				Ogre::Quaternion const &rot = Ogre::Quaternion::IDENTITY )
		: position(pos), quaternion(rot)
	{}

	Transform(Ogre::Quaternion const &rot)
		: position(Ogre::Vector3::ZERO), quaternion(rot)
	{}

	bool isIdentity(void) const;

	vl::Transform &operator*=(Ogre::Matrix4 const &m);

	vl::Transform &operator*=(vl::Transform const &t);

	Ogre::Vector3 position;
	Ogre::Quaternion quaternion;
};

std::ostream &
operator<<( std::ostream &os, Transform const &d );

/// Transform arithmetic
vl::Transform 
operator*(vl::Transform const &t1, vl::Transform const &t2);

/// Ogre::Matrix and Transform arithmetic
vl::Transform 
operator*( vl::Transform const &t, Ogre::Matrix4 const &m );

vl::Transform 
operator*( Ogre::Matrix4 const &m, vl::Transform const &t );

/// Comparison
bool
operator==(vl::Transform const &t1, vl::Transform const &t2);

inline bool
operator!=(vl::Transform const &t1, vl::Transform const &t2)
{
	return !(t1 == t2);
}

}	// namespace vl

#endif	// HYDRA_MATH_TRANSFORM_HPP