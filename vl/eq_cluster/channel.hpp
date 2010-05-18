#ifndef EQ_OGRE_CHANNEL_HPP
#define EQ_OGRE_CHANNEL_HPP

#include <eq/eq.h>
#include <eq/client/channel.h>
#include <eq/client/types.h>

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

		virtual void setCamera( vl::graph::CameraRefPtr cam );

		virtual void setViewport( vl::graph::ViewportRefPtr view );
		
    protected:
        virtual bool configInit( const uint32_t initID );
        virtual void frameClear( const uint32_t frameID );
        virtual void frameDraw( const uint32_t frameID );

 //       virtual void applyFrustum( void ) const;
    };
}

#endif // EQ_OGRE_CHANNEL_H

