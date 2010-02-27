/*	Joonatan Kuosa
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

#include "eq_graph/eq_render_window.hpp"

#include <OGRE/OgreRenderWindow.h>

#include "ogre_viewport.hpp"

namespace vl
{
namespace ogre 
{
	class RenderWindow : public vl::graph::RenderWindow
	{
		public :
			RenderWindow( Ogre::RenderWindow *win )
				: _ogre_window( win )
			{
				EQASSERT( _ogre_window );
			}

			virtual ~RenderWindow( void ) {}
			
			virtual Ogre::RenderWindow *getNative()
			{ return _ogre_window; }

			virtual void swapBuffers( void )
			{
				if( _ogre_window )
				{ _ogre_window->swapBuffers(); }
			}

			virtual void update( void )
			{
				if( _ogre_window )
				{ _ogre_window->update( false ); }
			}

			virtual vl::graph::Viewport *addViewport( vl::graph::Camera *cam )
			{
				Camera *c = (Camera *)cam;
				Ogre::Viewport *ogre_view
					= _ogre_window->addViewport( c->getNative() );
				Viewport *view = new Viewport( ogre_view );
				_viewports.push_back(view);
				return view;
			}

		protected :
			Ogre::RenderWindow *_ogre_window;
			std::vector<vl::graph::Viewport *> _viewports;

	};	// class RenderWindow

}	// namespace ogre

}	// namespace vl

#endif
