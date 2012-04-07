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

/// @class ObjectInterface
/// @todo Should be renamed object is such a stupid name, these are movable object (or node) types
class ObjectInterface
{
protected :
	typedef boost::signal<void (vl::Transform const &)> TransformedCB;
public :
	virtual ~ObjectInterface(void) {}

	virtual std::string const &getName(void) const = 0;

	virtual void transform(vl::Transform const &t) = 0;

	virtual void translate(Ogre::Vector3 const &v) = 0;

	void translate(vl::scalar x, vl::scalar y, vl::scalar z)
	{ translate(Ogre::Vector3(x, y, z)); }

	virtual void rotate(Ogre::Quaternion const &q) = 0;

	virtual void setWorldTransform(vl::Transform const &trans) = 0;

	/// @brief get the transformation in the world space
	/// @return Transformation in world space
	virtual vl::Transform getWorldTransform(void) const = 0;

	virtual Ogre::Vector3 const &getPosition(void) const = 0;

	virtual void setPosition(Ogre::Vector3 const &v) = 0;

	virtual Ogre::Quaternion const &getOrientation(void) const = 0;

	virtual void setOrientation(Ogre::Quaternion const &q) = 0;

	// @todo add transformation and rotation functions

	// @todo add clone

	// @todo add hide/show

	// @todo add callbacks for all changes
	virtual int addListener(TransformedCB::slot_type const &slot) = 0;

	void hide(void)
	{ setVisibility(false); }

	void show(void)
	{ setVisibility(true); }

	virtual void setVisibility(bool visible) = 0;

	virtual bool isVisible(void) const = 0;

	virtual void setShowBoundingBox(bool show) {}

	virtual bool getShowBoundingBox(void) const { return false; }

};	// class ObjectInterface

}	// namespace vl

#endif	// HYDRA_OBJECT_INTERFACE_HPP
