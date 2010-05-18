#ifndef EQ_OGRE_WINDOW_H
#define EQ_OGRE_WINDOW_H

#include <eq/client/window.h>

namespace eqOgre
{
	class Camera;

    /**
     * A window represent an OpenGL drawable and context
     *
     * Manages OpenGL-specific data, i.e., it creates the logo texture during
     * initialization and holds a state object for GL object creation. It
     * initializes the OpenGL state and draws the statistics overlay.
     */
    class Window : public eq::Window
    {
    public:
		Window( eq::Pipe *parent );

		virtual ~Window( void );
		
	protected :

		// Equalizer overrides
		virtual bool configInit( const uint32_t initID );
		virtual bool configInitGL( const uint32_t initID );
        virtual bool configExitGL( void );
        virtual void frameStart( const uint32_t frameID,
                                 const uint32_t frameNumber );
        virtual void swapBuffers( void );

    };	// class Window

}	// namespace eqOgre

#endif // EQ_OGRE_WINDOW_H
