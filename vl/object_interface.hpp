/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-09
 *	@file object_interface.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

/**
 *	Interface for all similar body objects e.g. RigidBody, KinematicBody and SceneNode.
 */

#ifndef HYDRA_OBJECT_INTERFACE_HPP
#define HYDRA_OBJECT_INTERFACE_HPP

#include "math/types.hpp"
#include "math/transform.hpp"

// Used for user callbacks
#include <boost/signal.hpp>

namespace vl
{

class ObjectInterface
{
protected :
	typedef boost::signal<void (vl::Transform const &)> TransformedCB;
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
	virtual int addListener(TransformedCB::slot_type const &slot) = 0;
};

}	// namespace vl

#endif	// HYDRA_OBJECT_INTERFACE_HPP
