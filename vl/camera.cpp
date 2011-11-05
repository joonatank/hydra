/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-04
 *	@file camera.cpp
 *
 */

#include "camera.hpp"

/// Necessary for creating the Ogre Camera
#include "scene_manager.hpp"
#include <OGRE/OgreSceneManager.h>

/// ------------------------------ Public ------------------------------------
vl::Camera::Camera(std::string const &name, vl::SceneManagerPtr creator)
	: MovableObject(name, creator)
{
	_clear();
}

/// Internal used by slave mapping
vl::Camera::Camera(vl::SceneManagerPtr creator)
	: MovableObject("", creator)
{
	_clear();
}

void 
vl::Camera::setNearClipDistance(Ogre::Real n)
{
	if( _near_clip != n )
	{
		setDirty(DIRTY_CLIPPING);
		_near_clip = n;
	}
}

void 
vl::Camera::setFarClipDistance(Ogre::Real n)
{
	if( _far_clip != n )
	{
		setDirty(DIRTY_CLIPPING);
		_far_clip = n;
	}
}

vl::MovableObjectPtr
vl::Camera::clone(std::string const &append_to_name) const
{
	assert(_creator);
	CameraPtr cam = _creator->createCamera(_name+append_to_name);
	cam->setFarClipDistance(_far_clip);
	cam->setNearClipDistance(_near_clip);
	cam->setPosition(_position);
	cam->setOrientation(_orientation);
	cam->setVisible(_visible);

	return cam;
}

/// ---------------------------- Protected -----------------------------------
void 
vl::Camera::doSerialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits ) const
{
	if( DIRTY_CLIPPING & dirtyBits )
	{
		msg << _near_clip << _far_clip;
	}
}

void
vl::Camera::doDeserialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits )
{
	if( DIRTY_CLIPPING & dirtyBits )
	{
		msg >> _near_clip >> _far_clip;
		if( _ogre_camera )
		{
			_ogre_camera->setNearClipDistance(_near_clip);
			_ogre_camera->setFarClipDistance(_far_clip);
		}
	}
}

bool 
vl::Camera::_doCreateNative(void)
{
	assert(_creator);
	assert(!_name.empty());

	if( _ogre_camera )
	{ return true; }

	_ogre_camera = _creator->getNative()->createCamera(_name);

	_ogre_camera->setNearClipDistance(_near_clip);
	_ogre_camera->setFarClipDistance(_far_clip);

	return true;
}

void 
vl::Camera::_clear(void)
{
	_position = Ogre::Vector3::ZERO;
	_orientation = Ogre::Quaternion::IDENTITY;
	_near_clip = 0.01;
	_far_clip = 100;
	_ogre_camera = 0;
}
