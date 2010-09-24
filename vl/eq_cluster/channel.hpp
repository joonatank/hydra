#ifndef EQ_OGRE_CHANNEL_HPP
#define EQ_OGRE_CHANNEL_HPP

#include <eq/eq.h>
#include <eq/client/channel.h>
#include <eq/client/types.h>

#include <OGRE/OgreCamera.h>
#include <OGRE/OgreCamera.h>
#include <OGRE/OgreViewport.h>
#include <OGRE/OgreRenderWindow.h>

#include "tracker.hpp"
#include "base/typedefs.hpp"

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

protected:
	// Equalizer overrides
    virtual bool configInit( const uint32_t initID );
    //virtual void frameClear( const uint32_t frameID );
    virtual void frameDraw( const uint32_t frameID );

	// Some task methods
	void setOgreFrustum( void );
	void setHeadMatrix( void );


//  virtual void applyFrustum( void ) const;
	Ogre::RenderWindow *_ogre_window;
	Ogre::Viewport *_viewport;
	Ogre::Camera *_camera;

	Ogre::Vector3 _head_pos;
	Ogre::Quaternion _head_orient;

	vl::TrackerRefPtr _tracker;

};

}

#endif // EQ_OGRE_CHANNEL_H

