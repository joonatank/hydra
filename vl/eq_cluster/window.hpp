#ifndef EQ_OGRE_WINDOW_H
#define EQ_OGRE_WINDOW_H

#include <eq/eq.h>
#include <eq/client/pipe.h>

//#include <OgreWindow.h>

//#include "vertexBufferState.h"
#include <string>

#include "eq_graph/eq_render_window.hpp"
#include "eq_graph/eq_camera.hpp"

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
		Window( eq::Pipe *parent )
			: eq::Window( parent ),
			  _state(0),
			  _render_window(0),
			  _camera(0)

		{}

		virtual ~Window( void )
		{}

		// Sets the Ogre::Camera projection matrix
		// called from Channel
		void setFrustum( vmml::mat4d const &m );
		
	protected :

		// Equalizer overrides
		virtual bool configInit( const uint32_t initID );
		virtual bool configInitGL( const uint32_t initID );
        virtual bool configExitGL( void );
        virtual void frameStart( const uint32_t frameID,
                                 const uint32_t frameNumber );
        virtual void swapBuffers( void );

		// Some helper functions
		void createViewports( void );

		//void createCameras( void );

		// TODO move to Node 
		// which should create the Window, camera and Viewport and set them
		// here using fifo_buffer.
		void createWindow( void );

		// TODO move to pipe, also needs to be checked with large file to see
		// if this will block (and deal with the blocking by changing it to
		// async function and providing callback).
		//
		// Also this function will become obsolete when we use at least two
		// frameStarted passes, first will finalize the initialisation
		// (e.g. create the windows and rendering system) and second
		// will create static objects (and after that we can start creating
		// dynamic objects).
		//
		// Just removing Client and processing commands in Node should fix the
		// issue (no entity should be created before the first frameStart pass).
		void loadMeshes( void );

    private:
		static unsigned int n_windows;

		unsigned int _state;

		vl::cl::RenderWindow *_render_window;
		vl::cl::Camera *_camera;
//		Ogre::Window *_ogre_window;
//		Ogre::Camera *_camera;

    };	// class Window

}	// namespace eqOgre

#endif // EQ_OGRE_WINDOW_H
