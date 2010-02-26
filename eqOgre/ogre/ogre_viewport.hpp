/*	Joonatan Kuosa
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

#include "../graph/viewport.hpp"

#include <OGRE/OgreViewport.h>

namespace vl
{
namespace ogre 
{
	class Viewport : public vl::graph::Viewport
	{
		public :
			Viewport( Ogre::Viewport *view )
				: _ogre_viewport( view )
			{
				EQASSERT( _ogre_viewport );
			}

			virtual Ogre::Viewport *getNative( void )
			{ return _ogre_viewport; }

			virtual ~Viewport( void )
			{}

			virtual void setBackgroundColour(
					vmml::vector<4, double> &colour )
			{
				if( _ogre_viewport )
				{
					Ogre::ColourValue c( colour.r(), colour.g(), colour.b(),
							colour.a() );
					_ogre_viewport->setBackgroundColour(c);
				}
			}

		protected :
			Ogre::Viewport *_ogre_viewport;

	};

}	// namespace graph

}	// namespace vl

#endif
