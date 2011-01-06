/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2011-01
 *	
 */

#ifndef EQ_OGRE_CHANNEL_HPP
#define EQ_OGRE_CHANNEL_HPP

#include <eq/eq.h>
#include <eq/client/channel.h>
#include <eq/client/types.h>

#include <OGRE/OgreCamera.h>
#include <OGRE/OgreViewport.h>

#include "eq_cluster/eq_settings.hpp"
#include "base/envsettings.hpp"

namespace eqOgre
{

/**
 * The rendering entity, updating a part of a Window.
 */
class Channel : public eq::Channel
{
public:
    Channel( eq::Window *parent );

    virtual ~Channel (void);

	DistributedSettings const &getSettings( void ) const;

	vl::Player const &getPlayer( void ) const;
	
	void setCamera( Ogre::Camera *cam );

	void setViewport( Ogre::Viewport *viewport );

	Ogre::Viewport *getViewport( void )
	{ return _viewport; }

protected:
	/// Equalizer overrides
	virtual bool configInit( const eq::uint128_t &initID );
	virtual bool configExit();

//	virtual void frameClear( const eq::uint128_t& frameID );

	/// Overrides all the equalizer frame draw methods
	/// Creating custom frustum and applying head matrix
	virtual void frameDraw( const eq::uint128_t &frameID );

	virtual void applyHeadTransform() const {}

	virtual void applyFrustum() const {}

//	virtual void frameFinish( const eq::uint128_t &frameID, uint32_t const frameNumber  );

//	virtual void frameViewFinish( const eq::uint128_t &frameID );

	// Some task methods
	// The eye is not a reference because it's going to get modified
	void setOgreFrustum( Ogre::Camera *camera, Ogre::Vector3 eye = Ogre::Vector3::ZERO );
	void setOgreView( Ogre::Camera *camera, Ogre::Vector3 eye = Ogre::Vector3::ZERO );
	
	/// Ogre variables
	Ogre::Viewport *_viewport;

	vl::EnvSettings::Wall _wall;

	bool _stereo;

};	// class Channel

}	// namespace eqOgre

#endif // EQ_OGRE_CHANNEL_H

