/**	Joonatan Kuosa
 *	2010-02
 *
 *	Container class for engine specific window.
 *	Used to create the window inside Node and transfer it to the
 *	rendering window.
 *
 *	Mind you rendering windows are equalizer specific. These windows are
 *	canvases used by the rendering engines to paint to.
 *	But as the engines have complex methods of creating these windows and
 *	need some predefined conditions for them and also the creation is not
 *	thread safe we need to transfer this like a part of scene graph.
 *
 *	Not distributed, so basicly not a part of the distributed graph.
 *	These windows are only used for interface to engine specific
 *	windows and are not user accessible.
 */
#ifndef VL_OGRE_RENDER_WINDOW_HPP
#define VL_OGRE_RENDER_WINDOW_HPP

// Base interface
#include "eq_graph/eq_render_window.hpp"

// Ogre includes
#include <OGRE/OgreRenderWindow.h>

namespace vl
{
namespace ogre 
{
	class RenderWindow : public vl::graph::RenderWindow
	{
		public :
			RenderWindow( Ogre::RenderWindow *win );

			virtual ~RenderWindow( void );
			
			virtual Ogre::RenderWindow *getNative();

			virtual void swapBuffers( void );

			virtual void update( void );

			virtual vl::graph::ViewportRefPtr
				addViewport( vl::graph::CameraRefPtr cam );

			virtual uint16_t getNumViewports( void ) const;

			virtual vl::graph::ViewportRefPtr getViewport( uint16_t index );

			virtual vl::graph::ViewportRefPtr getViewportByZOrder( int ZOrder );

			virtual bool hasViewportWithZOrder( int ZOrder );

			virtual void removeViewport( int ZOrder );

		protected :
			Ogre::RenderWindow *_ogre_window;
			std::vector<vl::graph::ViewportRefPtr> _viewports;

	};	// class RenderWindow

}	// namespace ogre

}	// namespace vl

#endif
