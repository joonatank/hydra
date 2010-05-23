
/**	Joonatan Kuosa
 *	2010-02
 *
 *	Container class for engine specific camera.
 *	Used to create the viewport inside a Node and transfer it to the
 *	rendering window.
 *
 *	Not distributed, so basicly not a part of the distributed graph.
 *	These Viewports are only used for interface to engine specific
 *	viewports and are not user accessible.
 */

#ifndef VL_OGRE_VIEWPORT_HPP
#define VL_OGRE_VIEWPORT_HPP

// Base interface
#include "interface/viewport.hpp"

// Ogre includes
#include <OGRE/OgreViewport.h>

namespace vl
{
namespace ogre 
{
	class Viewport : public vl::graph::Viewport
	{
		public :
			Viewport( Ogre::Viewport *view );

			virtual ~Viewport( void );

			virtual Ogre::Viewport *getNative( void );

			/// Rendering methods
			virtual void update( void );

			/// Clear the viewport to background colour
			virtual void clear( void );

			// TODO not implemented
			virtual vl::graph::RenderWindowRefPtr getTarget( void );

			// TODO not implemented
			virtual vl::graph::CameraRefPtr getCamera( void );

			// TODO not implemented
			virtual void setCamera( vl::graph::CameraRefPtr cam );

			virtual int getZOrder( void );

			/// Set the viewport background colour used for subsequent calls
			/// to clear.
			virtual void setBackgroundColour( vl::colour const &colour );

		protected :
			Ogre::Viewport *_ogre_viewport;
	};

}	// namespace graph

}	// namespace vl

#endif
