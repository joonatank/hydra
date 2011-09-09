/**	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-09
 *	@file animation/kinematic_body.cpp
 *
 *	This file is part of Hydra a VR game engine.
 *
 *	Interface for all similar body objects e.g. RigidBody, KinematicBody and SceneNode.
 */

#ifndef HYDRA_OBJECT_INTERFACE_HPP
#define HYDRA_OBJECT_INTERFACE_HPP

#include "math/types.hpp"
#include "math/transform.hpp"

namespace vl
{

class ObjectInterface
{
public :
	virtual ~ObjectInterface(void) {}

	virtual std::string const &getName(void) const = 0;

	virtual void setWorldTransform(vl::Transform const &trans) = 0;

	/// @brief get the transformation in the world space
	/// @return Transformation in world space
	virtual vl::Transform getWorldTransform(void) const = 0;

	// @todo add transformation and rotation functions

	// @todo add clone

	// @todo add hide/show

	// @todo add callbacks for all changes
};

}	// namespace vl

#endif	// HYDRA_OBJECT_INTERFACE_HPP
