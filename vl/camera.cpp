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

void 
vl::Camera::setPosition(Ogre::Vector3 const &pos)
{
	if( _position != pos )
	{
		setDirty(DIRTY_POSITION);
		_position = pos;
	}
}

void 
vl::Camera::setOrientation(Ogre::Quaternion const &q)
{
	if( _orientation != q )
	{
		setDirty(DIRTY_ORIENTATION);
		_orientation = q;
	}
}

/// ---------------------------- Protected -----------------------------------
void 
vl::Camera::doSerialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits )
{
	if( DIRTY_CLIPPING & dirtyBits )
	{
		msg << _near_clip << _far_clip;
	}

	if( DIRTY_POSITION & dirtyBits )
	{
		msg << _position;
	}

	if( DIRTY_ORIENTATION & dirtyBits )
	{
		msg << _orientation;
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

	if( DIRTY_POSITION & dirtyBits )
	{
		msg >> _position;
		if( _ogre_camera )
		{ _ogre_camera->setPosition(_position); }
	}

	if( DIRTY_ORIENTATION & dirtyBits )
	{
		msg >> _orientation;
		if( _ogre_camera )
		{ _ogre_camera->setOrientation(_orientation); }
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
	_ogre_camera->setPosition(_position);
	_ogre_camera->setOrientation(_orientation);

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
