/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-12
 *
 *	Player class
 *	Manages the player in the Game world
 *	Contains current active camera
 *	Contains current camera target
 *	Contains all the possible cameras player can use and their targets
 *	Set the head matrix for tracking
 *
 *	Non copyable as there should only be one of these at the moment
 *
 */

#ifndef VL_PLAYER_HPP
#define VL_PLAYER_HPP

#include <string>

#include <OGRE/OgreMatrix4.h>

#include <eq/client/observer.h>

namespace vl
{

class Player
{
public :
	Player( eq::Observer *observer );

	virtual ~Player( void );

	void setActiveCamera( std::string const &name );

	std::string const &getActiveCamera( void ) const;

	void setHeadMatrix( Ogre::Matrix4 const &m );

private :
	/// Non copyable
	Player( Player const &);
	Player & operator=( Player const &);

	std::string _active_camera;

	eq::Observer *_observer;

};	// class Player

}	// namespace vl

#endif // VL_PLAYER_HPP