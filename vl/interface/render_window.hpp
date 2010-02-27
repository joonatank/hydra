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
#ifndef VL_GRAPH_WINDOW_HPP
#define VL_GRAPH_WINDOW_HPP

#include "viewport.hpp"
#include "camera.hpp"

namespace vl
{
namespace graph
{
	class RenderWindow
	{
		public :
			RenderWindow( void ) {}

			virtual ~RenderWindow( void ) {}
			
			virtual void swapBuffers( void ) = 0;

			virtual void update( void ) = 0;

			virtual Viewport *addViewport( Camera *cam ) = 0;

		protected :
	};

}

}

#endif
