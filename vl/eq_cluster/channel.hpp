/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2011-01
 *
 */

#ifndef EQ_OGRE_CHANNEL_HPP
#define EQ_OGRE_CHANNEL_HPP

#include <OGRE/OgreCamera.h>
#include <OGRE/OgreViewport.h>

#include "base/envsettings.hpp"

namespace eqOgre
{

class Window;

/**
 * The rendering entity, updating a part of a Window.
 */
class Channel
{
public:
    Channel( eqOgre::Window *parent );

    virtual ~Channel (void);

// 	DistributedSettings const &getSettings( void ) const;

	vl::Player const &getPlayer( void ) const;

	void setCamera( Ogre::Camera *cam );

	void setViewport( Ogre::Viewport *viewport );

	Ogre::Viewport *getViewport( void )
	{ return _viewport; }

	std::string getName( void ) const
	{ return std::string(); }

protected:
	/// Equalizer overrides
	virtual bool configInit( uint64_t initID );
	virtual bool configExit();

	/// Overrides all the equalizer frame draw methods
	/// Creating custom frustum and applying head matrix
	virtual void frameDraw( uint64_t frameID );

	// Some task methods
	// The eye is not a reference because it's going to get modified
	void setOgreFrustum( Ogre::Camera *camera, Ogre::Vector3 eye = Ogre::Vector3::ZERO );
	void setOgreView( Ogre::Camera *camera, Ogre::Vector3 eye = Ogre::Vector3::ZERO );

	eqOgre::Window *_window;

	/// Ogre variables
	Ogre::Viewport *_viewport;

	vl::EnvSettings::Wall _wall;

	bool _stereo;

};	// class Channel

}	// namespace eqOgre

#endif // EQ_OGRE_CHANNEL_H

