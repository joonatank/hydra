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

#include "interface/viewport.hpp"

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
				if( !_ogre_viewport )
				{ throw vl::null_pointer( "vl::ogre::Camera::Camera" ); }
			}

			virtual ~Viewport( void )
			{}

			virtual Ogre::Viewport *getNative( void )
			{ return _ogre_viewport; }

			// Rendering methods
			virtual void update( void ) 
			{
				_ogre_viewport->update();
			}

			virtual void clear( void )
			{
				_ogre_viewport->clear();
			}

			virtual vl::graph::RenderWindowRefPtr getTarget( void )
			{
			//	return _ogre_viewport->getTarget();
				return vl::graph::RenderWindowRefPtr();
			}

			virtual vl::graph::CameraRefPtr getCamera( void )
			{
			//	return _ogre_viewport->getCamera();
				return vl::graph::CameraRefPtr();
			}

			virtual void setCamera( vl::graph::CameraRefPtr cam )
			{
			//	_ogre_viewport->setCamera(cam);
			}

			virtual int getZOrder( void )
			{
				return _ogre_viewport->getZOrder();
			}

			virtual void setBackgroundColour( vl::colour const &colour )
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
