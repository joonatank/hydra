/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2011-01
 *
 *	2011-02 Update
 *	Removed Equalizer
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
    Channel( vl::EnvSettings::Channel chanConf, eqOgre::Window *parent );

    virtual ~Channel (void);

	vl::Player const &getPlayer( void ) const;

	void setCamera( Ogre::Camera *cam );

	void setViewport( Ogre::Viewport *viewport );

	Ogre::Viewport *getViewport( void )
	{ return _viewport; }

	std::string getName( void ) const
	{ return _channel_conf.name; }

	virtual void draw( void );

protected:
	virtual void init( vl::EnvSettingsRefPtr settings );

	// Some task methods
	// The eye is not a reference because it's going to get modified
	void _setOgreFrustum( Ogre::Camera *camera, Ogre::Vector3 eye = Ogre::Vector3::ZERO );
	void _setOgreView( Ogre::Camera *camera, Ogre::Vector3 eye = Ogre::Vector3::ZERO );

	eqOgre::Window *_window;

	/// Ogre variables
	Ogre::Viewport *_viewport;

	vl::EnvSettings::Wall _wall;

	bool _stereo;

	vl::EnvSettings::Channel _channel_conf;

};	// class Channel

}	// namespace eqOgre

#endif // EQ_OGRE_CHANNEL_H

