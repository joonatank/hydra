/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-12
 *
 *	Player class
 *	Manages the player in the Game world
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

#ifndef VL_PLAYER_HPP
#define VL_PLAYER_HPP

#include <string>

#include "math/math.hpp"

#include "distributed.hpp"

namespace vl
{

class Player : public vl::Distributed
{
public :
	/// Constructor
	Player( void );

	virtual ~Player( void );

	void setActiveCamera( std::string const &name );

	std::string const &getActiveCamera( void ) const
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
	virtual void serialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits );
	virtual void deserialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits );

private :
	/// Non copyable
	Player( Player const &);
	Player & operator=( Player const &);

	std::string _active_camera;

	vl::Transform _head_transform;

	uint32_t _screenshot_version;

	double _ipd;

};	// class Player

}	// namespace vl

#endif // VL_PLAYER_HPP
