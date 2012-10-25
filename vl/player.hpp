/**
 *	Copyright (c) 2010 - 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-12
 *	@file player.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

/**
 *
 *	Player class
 *	Manages the player or observer in the Game world
 *	Contains current active camera
 *	Contains current camera target
 *	Contains all the possible cameras player can use and their targets
 *	Contains the head matrix and distributes it to Channels.
 *
 *	Non copyable as there should only be one of these at the moment
 *
 *	2011-01 Updated:
 *	Removed Equalizer observer.
 *	We are using our own code for setting the head matrix player only distributes.
 *
 *	Removed dependency to Equalizer.
 */

#ifndef HYDRA_PLAYER_HPP
#define HYDRA_PLAYER_HPP

// Necessary for HYDRA_API
#include "defines.hpp"

#include "math/transform.hpp"

#include "cluster/distributed.hpp"

#include "typedefs.hpp"

#include <string>

namespace vl
{

class HYDRA_API Player : public vl::Distributed
{
public :
	/// @brief Constructor
	/// @param scene_manager used for retrieving the camera
	Player(SceneManagerPtr scene_manager);

	virtual ~Player( void );

	/// Depricated Camera functions
	void setActiveCamera( std::string const &name );

	std::string const &getActiveCamera(void) const;

	// New functions, will replace the old name based
	void setCamera(CameraPtr cam);

	CameraPtr getCamera(void) const
	{ return _active_camera; }

	SceneNodePtr getCameraNode(void) const;

	vl::Transform const &getHeadTransform(void) const
	{ return _head_transform; }

	void setHeadTransform(vl::Transform const &m);

	vl::Transform const &getCyclopTransform(void) const
	{ return _cyclop_transform; }

	void setCyclopTransform(vl::Transform const &t);

	vl::Transform getCyclopWorldTransform(void) const;

	void takeScreenshot( void );

	uint32_t getScreenshotVersion( void ) const
	{ return _screenshot_version; }

	void setIPD(double ipd);

	double getIPD(void) const
	{ return _ipd; }

	enum DirtyBits
	{
		DIRTY_HEAD = vl::Distributed::DIRTY_CUSTOM << 0,
		DIRTY_CYCLOP = vl::Distributed::DIRTY_CUSTOM << 1,
		DIRTY_ACTIVE_CAMERA = vl::Distributed::DIRTY_CUSTOM << 2,
		DIRTY_SCREENSHOT = vl::Distributed::DIRTY_CUSTOM << 3,
		DIRTY_IPD = vl::Distributed::DIRTY_CUSTOM << 4,
		DIRTY_FRUSTUM = vl::Distributed::DIRTY_CUSTOM << 5,
		DIRTY_CUSTOM = vl::Distributed::DIRTY_CUSTOM << 6,
	};

private :
	virtual void serialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits ) const;
	virtual void deserialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits );

	/// Non copyable
	Player( Player const &);
	Player & operator=( Player const &);

	CameraPtr _active_camera;

	vl::Transform _head_transform;

	vl::Transform _cyclop_transform;

	uint32_t _screenshot_version;

	double _ipd;

	SceneManagerPtr _scene_manager;

};	// class Player

std::ostream &operator<<(std::ostream &os, Player const &player);

}	// namespace vl

#endif	// HYDRA_PLAYER_HPP
