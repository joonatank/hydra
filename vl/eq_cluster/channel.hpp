#ifndef EQ_OGRE_CHANNEL_HPP
#define EQ_OGRE_CHANNEL_HPP

#include <eq/eq.h>
#include <eq/client/channel.h>
#include <eq/client/types.h>

#include <OgreCamera.h>
#include <OgreCamera.h>
#include <OgreViewport.h>
#include <OgreRenderWindow.h>

#include "tracker.hpp"
#include "base/typedefs.hpp"
#include "frame_data.hpp"

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

	void updateDistribData( void );

//  virtual void applyFrustum( void ) const;
	Ogre::RenderWindow *_ogre_window;
	Ogre::Viewport *_viewport;
	Ogre::Camera *_camera;

	// distribution related
	FrameData _frame_data;

	// Tracker stuff
	Ogre::Vector3 _head_pos;
	Ogre::Quaternion _head_orient;

	vl::TrackerRefPtr _tracker;

};

}

#endif // EQ_OGRE_CHANNEL_H

