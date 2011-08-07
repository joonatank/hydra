/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2010-12
 *	@file player.hpp
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

#include <string>

#include "math/transform.hpp"

#include "distributed.hpp"

#include "typedefs.hpp"

namespace vl
{

class Player : public vl::Distributed
{
public :
	/// @brief Constructor
	/// @param scene_manager used for retrieving the camera
	Player(SceneManagerPtr scene_manager);

	virtual ~Player( void );

	void setActiveCamera( std::string const &name );

	std::string const &getActiveCamera(void) const;

	// New functions, should in the long run replace the name retrieval

	void setCamera(CameraPtr cam);

	CameraPtr getCamera(void) const
	{ return _active_camera; }

	vl::Transform const &getHeadTransform(void) const
	{ return _head_transform; }

	void setHeadTransform(vl::Transform const &m);

	void takeScreenshot( void );

	uint32_t getScreenshotVersion( void ) const
	{ return _screenshot_version; }

	void setIPD(double ipd);

	double getIPD(void) const
	{ return _ipd; }

	enum DirtyBits
	{
		DIRTY_HEAD = vl::Distributed::DIRTY_CUSTOM << 0,
		DIRTY_ACTIVE_CAMERA = vl::Distributed::DIRTY_CUSTOM << 1,
		DIRTY_SCREENSHOT = vl::Distributed::DIRTY_CUSTOM << 2,
		DIRTY_IPD = vl::Distributed::DIRTY_CUSTOM << 3,
		DIRTY_CUSTOM = vl::Distributed::DIRTY_CUSTOM << 4,
	};

protected :
	virtual void serialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits ) const;
	virtual void deserialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits );

private :
	/// Non copyable
	Player( Player const &);
	Player & operator=( Player const &);

	CameraPtr _active_camera;

	vl::Transform _head_transform;

	uint32_t _screenshot_version;

	double _ipd;

	SceneManagerPtr _scene_manager;

};	// class Player

std::ostream &operator<<(std::ostream &os, Player const &player);

}	// namespace vl

#endif	// HYDRA_PLAYER_HPP
