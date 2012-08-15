/**
 *	Copyright (c) 2010 - 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-12
 *	@file player.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

#include "player.hpp"

/// Necessary for retrieving the camera
#include "scene_manager.hpp"
/// Necessary for manipulating active camera
#include "camera.hpp"

/// ------------------------------ Global ------------------------------------
std::ostream &
vl::operator<<(std::ostream &os, vl::Player const &player)
{
	os << "Player : active camera = \"" << player.getActiveCamera() << "\"" 
		<< " : ipd = " << player.getIPD() 
		<< " : head transform = " << player.getHeadTransform() << std::endl;

	return os;
}

/// ------------------------------ Player ------------------------------------
vl::Player::Player(SceneManagerPtr scene_manager)
	: _active_camera(0)
	, _screenshot_version(0)
	, _ipd(0)
	, _scene_manager(scene_manager)
{
	assert(_scene_manager);
	_active_camera = _scene_manager->getCamera(EDITOR_CAMERA);
	// Master copy has to have the camera, but we can't assert here because
	// the slave copy does not have it.
}

vl::Player::~Player( void )
{}

void
vl::Player::setActiveCamera(std::string const &name)
{
	if(!_active_camera || (_active_camera->getName() != name))
	{
		if(_scene_manager->hasCamera(name))
		{
			_active_camera = _scene_manager->getCamera(name);
			setDirty(DIRTY_ACTIVE_CAMERA);
		}
		else
		{
			std::cout << "Couldn't set active camera because Camera \""
				<< name << "\" doesn't exists." << std::endl;
		}
	}
}

void
vl::Player::setCamera(vl::CameraPtr cam)
{
	if(_active_camera != cam)
	{
		setDirty(DIRTY_ACTIVE_CAMERA);
		_active_camera = cam;
	}
}


std::string const &
vl::Player::getActiveCamera(void) const
{
	assert(_active_camera);
	return _active_camera->getName();
}

vl::SceneNodePtr
vl::Player::getCameraNode(void) const
{
	if(_active_camera)
	{ return _active_camera->getParent(); }

	return 0;
}


void
vl::Player::setHeadTransform(vl::Transform const &t)
{
	setDirty(DIRTY_HEAD);
	_head_transform = t;
}

void
vl::Player::setCyclopTransform(vl::Transform const &t)
{
	if(t != _cyclop_transform)
	{
		setDirty(DIRTY_CYCLOP);
		_cyclop_transform = t;
	}
}

vl::Transform
vl::Player::getCyclopWorldTransform(void) const
{
	return _head_transform*_cyclop_transform;
}

void
vl::Player::takeScreenshot( void )
{
	setDirty( DIRTY_SCREENSHOT );
	_screenshot_version++;
}

void
vl::Player::setIPD(double ipd)
{
	if( _ipd != ipd )
	{
		setDirty(DIRTY_IPD);
		_ipd = ipd;
	}
}


/// ------------------------------- Private  ---------------------------------
void
vl::Player::serialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits ) const
{
	if( dirtyBits & DIRTY_HEAD )
	{ msg << _head_transform; }

	if( dirtyBits & DIRTY_CYCLOP )
	{ msg << _cyclop_transform; }

	if( dirtyBits & DIRTY_ACTIVE_CAMERA )
	{
		assert(_active_camera);
		msg << _active_camera->getID();
	}

	if( dirtyBits & DIRTY_SCREENSHOT )
	{ msg << _screenshot_version; }

	if( dirtyBits & DIRTY_IPD )
	{ msg << _ipd; }
}

void
vl::Player::deserialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits )
{
	if( dirtyBits & DIRTY_HEAD )
	{ msg >> _head_transform; }

	if( dirtyBits & DIRTY_CYCLOP )
	{ msg >> _cyclop_transform; }

	if( dirtyBits & DIRTY_ACTIVE_CAMERA )
	{
		uint64_t id;
		msg >> id;

		assert(_scene_manager);
		assert(dynamic_cast<CameraPtr>(_scene_manager->getMovableObjectID(id)));
		_active_camera = static_cast<CameraPtr>(_scene_manager->getMovableObjectID(id));
		assert(_active_camera);
	}

	if( dirtyBits & DIRTY_SCREENSHOT )
	{ msg >> _screenshot_version; }

	if( dirtyBits & DIRTY_IPD )
	{ msg >> _ipd; }
}
