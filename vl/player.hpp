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
 */

#ifndef VL_PLAYER_HPP
#define VL_PLAYER_HPP

#include <string>

#include <OGRE/OgreMatrix4.h>

#include <eq/fabric/serializable.h>

// TODO should be moved to eq_cluster because this depends on the Equalizer

namespace vl
{

class Player : public eq::fabric::Serializable
{
public :
	/// Constructor
	Player( void );

	virtual ~Player( void );

	void setActiveCamera( std::string const &name );

	std::string const &getActiveCamera( void ) const;

	Ogre::Matrix4 const &getHeadMatrix( void ) const
	{ return _head_matrix; }

	void setHeadMatrix( Ogre::Matrix4 const &m );

	void takeScreenshot( void );

	uint32_t getScreenshotVersion( void ) const
	{ return _screenshot_version; }

	enum DirtyBits
	{
		DIRTY_HEAD = eq::fabric::Serializable::DIRTY_CUSTOM << 0,
		DIRTY_ACTIVE_CAMERA = eq::fabric::Serializable::DIRTY_CUSTOM << 1,
		DIRTY_SCREENSHOT = eq::fabric::Serializable::DIRTY_CUSTOM << 2,
	};

protected :
	virtual void serialize( co::DataOStream &os, const uint64_t dirtyBits );
	virtual void deserialize( co::DataIStream &is, const uint64_t dirtyBits );

private :
	/// Non copyable
	Player( Player const &);
	Player & operator=( Player const &);

	std::string _active_camera;

	Ogre::Matrix4 _head_matrix;

	uint32_t _screenshot_version;

};	// class Player

}	// namespace vl

#endif // VL_PLAYER_HPP
