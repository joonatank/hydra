/**
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-09
 *	@file object_interface.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
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

#include "typedefs.hpp"

#include "defines.hpp"

// Used for user callbacks
#include <boost/signal.hpp>

namespace vl
{

enum TransformSpace
{
	TS_LOCAL,
	TS_PARENT,
	TS_WORLD,
};

/// @class ObjectInterface
/// @todo Should be renamed object is such a stupid name, these are movable object (or node) types
class HYDRA_API ObjectInterface
{
protected :
	typedef boost::signal<void (vl::Transform const &)> TransformedCB;
public :
	virtual ~ObjectInterface(void) {}

	virtual std::string const &getName(void) const = 0;


	/// Transformations
	
	// Base virtuals
	virtual void setTransform(Transform const &t) = 0;
	virtual Transform const &getTransform(void) const = 0;

	virtual void setWorldTransform(vl::Transform const &trans) = 0;

	/// @brief get the transformation in the world space
	/// @return Transformation in world space
	virtual vl::Transform getWorldTransform(void) const = 0;

	/// @brief set the transformation in the space of an another object
	/// @param trans Transformation in reference space
	/// @param reference the space to use for Transformation
	/// @todo not testes, and probably does not work correctly
	void setTransform(vl::Transform const &trans, ObjectInterfacePtr reference);

	/// @brief get transformation in the space of an another object
	/// @param reference the space the Transformation is returned
	/// @return Transformation in reference space
	/// @todo not testes, and probably does not work correctly
	vl::Transform getTransform(ObjectInterfacePtr reference) const;

	/// @brief get the parent objects world transformation
	/// This is really non optimised way of doing it though
	/// Provided for convience and faster ways would require either
	/// expose more functions or create specialised versions in inherited classes.
	/// The difference arises from the fact that some objects don't have
	/// parents and some do and we only have access to their 
	/// world and local transformations.
	Transform getParentWorldTransform(void) const;

	Ogre::Vector3 const &getPosition(void) const
	{ return getTransform().position; }

	void setPosition(Ogre::Vector3 const &v)
	{ setTransform(Transform(getTransform().quaternion, v)); }

	Ogre::Quaternion const &getOrientation(void) const
	{ return getTransform().quaternion; }

	void setOrientation(Ogre::Quaternion const &q)
	{ setTransform(Transform(q, getTransform().position)); }


	// Conveniance functions for transformations
	
	/// @brief transform the node, does not include the scale part
	void transform(vl::Transform const &t);

	/// @brief transform the node by a matrix, includes the scale part
	/// @param m matrix to use for transformation
	void transform(Ogre::Matrix4 const &m);

	/// @brief set the transformation of the node by a matrix, includes the scale part
	/// @param m matrix to replace the transformation
	void setTransform(Ogre::Matrix4 const &m);


	void translate(Ogre::Vector3 const &v);

	void translate(vl::scalar x, vl::scalar y, vl::scalar z)
	{ translate(Ogre::Vector3(x, y, z)); }

	/// @brief translate the SceneNode
	/// @param v how much to translate
	/// @param reference the coordinate system to use for translation
	void translate(Ogre::Vector3 const &v, vl::ObjectInterfacePtr reference);

	/// @brief translate the SceneNode
	/// @param v how much to translate
	/// @param which coordinate system to use
	void translate(Ogre::Vector3 const &v, vl::TransformSpace space);

	/// @brief rotates the SceneNode in transform space
	/// @param q how much to rotate
	/// @param which coordinate system to use
	void rotate(Ogre::Quaternion const &q, vl::TransformSpace space);

	/// @brief rotates the SceneNode using World axes
	virtual void rotate(Ogre::Quaternion const &q);

	/// @brief helper overloads
	void rotate(Ogre::Vector3 const &axis, Ogre::Radian const &angle)
	{ rotate(Ogre::Quaternion(angle, axis)); }
	void rotate(Ogre::Radian const &angle, Ogre::Vector3 const &axis)
	{ rotate(Ogre::Quaternion(angle, axis)); }
	void rotate(Ogre::Vector3 const &axis, Ogre::Degree const &angle)
	{ rotate(Ogre::Quaternion(Ogre::Radian(angle), axis)); }
	void rotate(Ogre::Degree const &angle, Ogre::Vector3 const &axis)
	{ rotate(Ogre::Quaternion(Ogre::Radian(angle), axis)); }

	/// @todo properly document the rotation functions and their uses

	/// @brief rotates the SceneNode around reference
	/// @param q how much to rotate
	/// @param reference the coordinate system to use for translation
	/// @todo seems like this doesn't not work as expected for some rotations
	/// @todo should this be renamed to rotateAround, because this is only useful for special cases
	void rotateAround(Ogre::Quaternion const &q, ObjectInterfacePtr reference);

//	void rotateAround(Ogre::Quaternion const &q, Ogre::Vector3 const &point, 
//		TransformSpace relativeTo = TS_LOCAL);


	void lookAt(Ogre::Vector3 const &dir);

	Ogre::Vector3 getDirection(void) const;

	/// @todo move the yawFixed and upAxis to class variables
	void setDirection(Ogre::Vector3 const &dir, 
		Ogre::Vector3 const &localDirectionVector = Ogre::Vector3::NEGATIVE_UNIT_Z, 
		Ogre::Vector3 const &upAxis = Ogre::Vector3::UNIT_Y, bool yawFixed = true);



	// @todo add clone

	// @todo add callbacks for all changes
	virtual int addListener(TransformedCB::slot_type const &slot) = 0;

	virtual void scale(Ogre::Vector3 const &v) = 0;

	virtual void setScale(Ogre::Vector3 const &v) = 0;

	virtual Ogre::Vector3 const &getScale(void) const = 0;

	/// Visibility
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
