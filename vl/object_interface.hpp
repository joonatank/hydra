/**
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-09
 *	@file object_interface.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
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

	/// @brief Is the object created by run time using a script
	/// @return True if the dynamic flag is set.
	/// Technically this can be true for any object but practically
	/// it should be true only for objects created from python.
	/// Allows some objects to be cleared without destroying the scene.
	/// Dynamic objects are also ignored when saving the scene.
	virtual bool isDynamic(void) const = 0;

};	// class ObjectInterface

}	// namespace vl

#endif	// HYDRA_OBJECT_INTERFACE_HPP
