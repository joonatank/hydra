/**
 *	Copyright (c) 2011 Tampere University of Technology
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-04
 *	@file camera.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef HYDRA_CAMERA_HPP
#define HYDRA_CAMERA_HPP

// Base class
#include "movable_object.hpp"

#include <OGRE/OgreCamera.h>

namespace vl
{

class Camera : public MovableObject
{
public :
	Camera(std::string const &name, vl::SceneManagerPtr creator);

	/// Internal used by slave mapping
	Camera(vl::SceneManagerPtr creator);

	void setNearClipDistance(Ogre::Real n);

	Ogre::Real getNearClipDistance(void) const
	{ return _near_clip; }

	void setFarClipDistance(Ogre::Real n);

	Ogre::Real getFarClipDistance(void) const
	{ return _far_clip; }

	void setPosition(Ogre::Vector3 const &pos);

	Ogre::Vector3 const &getPosition(void) const
	{ return _position; }

	void setOrientation(Ogre::Quaternion const &q);

	Ogre::Quaternion const &getOrientation(void) const
	{ return _orientation; }

	virtual MovableObjectPtr clone(std::string const &append_to_name) const;

	enum DirtyBits
	{
		DIRTY_CLIPPING = vl::MovableObject::DIRTY_CUSTOM << 0,
		DIRTY_TRANSFORM = vl::MovableObject::DIRTY_CUSTOM << 1,
		DIRTY_CUSTOM = vl::MovableObject::DIRTY_CUSTOM << 2,
	};

/// Virtual overrides
	virtual Ogre::MovableObject *getNative(void) const
	{ return _ogre_camera; }

	std::string getTypeName(void) const
	{ return "Camera"; }

private :

	virtual void doSerialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits ) const;
	virtual void doDeserialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits );

	virtual bool _doCreateNative(void);

	/// clears the structure to default values, called from constructors
	void _clear(void);

	Ogre::Vector3 _position;
	Ogre::Quaternion _orientation;
	Ogre::Real _near_clip;
	Ogre::Real _far_clip;

	Ogre::Camera *_ogre_camera;

};	// class Camera

inline std::ostream &
operator<<(std::ostream &os, vl::Camera const &cam)
{
	os << "Camera : " << cam.getName() << " : position " << cam.getPosition() 
		<< " : orientation " << cam.getOrientation() 
		<< " : near clip " << cam.getNearClipDistance()
		<< " : far clip " << cam.getFarClipDistance()
		<< std::endl;

	return os;
}

}	// namespace vl

#endif	// HYDRA_CAMERA_HPP