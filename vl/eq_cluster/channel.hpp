#ifndef EQ_OGRE_CHANNEL_HPP
#define EQ_OGRE_CHANNEL_HPP

#include <OgreViewport.h>

#include <eq/eq.h>
#include <eq/client/channel.h>
#include <eq/client/types.h>

//#include "camera.hpp"

namespace eqOgre
{
//	class RenderWindow;
//	class Root;
//    class FrameData;
//    class InitData;

    /**
     * The rendering entity, updating a part of a Window.
     */
    class Channel : public eq::Channel//, public Ogre::Viewport
    {
    public:
        Channel( eq::Window *parent );

        virtual ~Channel (void)
		{}

//		void setCamera( eqOgre::Camera *cam );

//		void setViewport( Ogre::Viewport *viewport );

    protected:
        virtual bool configInit( const uint32_t initID );
        virtual void frameClear( const uint32_t frameID );
        virtual void frameDraw( const uint32_t frameID );
        virtual void frameReadback( const uint32_t frameID );
        virtual void frameViewFinish( const uint32_t frameID );

        /** Applies the perspective or orthographic frustum. */
        virtual void applyFrustum (void) const;

//		InitData initData;
//		eqOgre::Root *_root;
//		Ogre::Viewport *_ogre_viewport;
//		eqOgre::Camera *_camera;
//		void _drawModel( const Model* model );
//		void _drawOverlay (void);
//		void _drawHelp (void);

//		void _updateNearFar ( const mesh::BoundingSphere& boundingSphere );
//		void _initFrustum ( eq::FrustumCullerf& frustum, 
//						   const mesh::BoundingSphere& boundingSphere );

//		const FrameData &_getFrameData (void) const;
//		const Model *_getModel (void);

//		const Model *_model;
//		uint32_t _modelID;
    };
}

#endif // EQ_OGRE_CHANNEL_H

