/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011/10 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-05
 *	@file math/transform.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef HYDRA_MATH_TRANSFORM_HPP
#define HYDRA_MATH_TRANSFORM_HPP

// Necessary for vl::scalar, vector, quaternion and matrix types
#include "math/types.hpp"

namespace vl
{

struct Transform
{
	/// Constructors allows for defining Vector and Quaternion pair in any order
	Transform( Ogre::Vector3 const &pos = Ogre::Vector3::ZERO,
				Ogre::Quaternion const &rot = Ogre::Quaternion::IDENTITY )
		: position(pos), quaternion(rot)
	{}

	Transform(Ogre::Quaternion const &rot, Ogre::Vector3 const &pos = Ogre::Vector3::ZERO)
		: position(pos), quaternion(rot)
	{}

	bool isValid(void) const
	{ return !quaternion.isNaN() && !position.isNaN(); }

	bool isIdentity(void) const;

	void setIdentity(void);

	bool isPositionZero(void) const;

	void setPositionZero(void);

	bool isRotationIdentity(void) const;

	void setRotationIdentity(void);

	/// @brief inverts the transformation
	void invert(void);

	vl::Transform inverted(void) const
	{
		vl::Transform t(*this);
		t.invert();
		return t;
	}

	Ogre::Vector3 rotate(Ogre::Vector3 const &v) const;

	vl::Transform &operator*=(Ogre::Matrix4 const &m);

	vl::Transform &operator*=(vl::Transform const &t);

	/// Does not define T+T and T-T because those are meaningless for Transformations
	/// and would screw quaternions for sure
	/// The multiplication operator is provided and is quarantied not to
	/// touch the quaternion.
	vl::Transform &operator*=(Ogre::Vector3 const &v);

	vl::Transform &operator*=(vl::scalar s);

	Ogre::Vector3 position;
	Ogre::Quaternion quaternion;
};

std::ostream &
operator<<( std::ostream &os, Transform const &d );

/// Transform arithmetic
// Invert the transformation, returns a version with Quaternion inverted and
// Vector negated so that T*(-T) = (-T)*T = I for all Ts.
vl::Transform
operator-(vl::Transform const &t);

vl::Transform 
operator*(vl::Transform const &t1, vl::Transform const &t2);


/// Ogre::Matrix and Transform arithmetic
vl::Transform 
operator*( vl::Transform const &t, Ogre::Matrix4 const &m );

vl::Transform 
operator*( Ogre::Matrix4 const &m, vl::Transform const &t );

/// transforms the vector by transfrom t
/// Does both, rotates the vector by the quaternion and adds the positions element
/// Equivalent to Matrix Vector multiplication
Ogre::Vector3
operator*( vl::Transform const &t, Ogre::Vector3 const &v);

/// Transforms quaternion by Transform, only rotation part is considered
/// Equivalent to Matrix Rotatation matrix (quaternion) multiplication
Ogre::Quaternion
operator*( vl::Transform const &t, Ogre::Quaternion const &q);




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
