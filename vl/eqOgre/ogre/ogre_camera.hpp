/*	Joonatan Kuosa
 *	2010-02
 *
 *	Container class for engine specific camera.
 *	Used to create the camera inside Node and transfer it to the
 *	rendering window.
 *
 *	Not distributed, so basicly not a part of the distributed graph.
 *	These cameras are only used for interface to engine specific
 *	cameras and are not user accessible.
 */
#ifndef VL_OGRE_CAMERA_HPP
#define VL_OGRE_CAMERA_HPP

#include "../graph/camera.hpp"

#include <OGRE/OgreCamera.h>

namespace vl
{
namespace ogre 
{
	class Camera : public vl::graph::Camera
	{
		public :
			Camera( Ogre::Camera *cam )
				: _ogre_camera(cam)
			{
				EQASSERT( _ogre_camera );
			}

			virtual ~Camera( void ) {}

			Ogre::Camera *getNative( void )
			{ return _ogre_camera; }

			virtual void setProjectionMatrix( vmml::mat4d const &m )
			{
				if( _ogre_camera )
				{
					_ogre_camera->setCustomProjectionMatrix( true,
							vl::math::convert(m) );
				}
			}

		protected :
			Ogre::Camera *_ogre_camera;

	};	// class Camera

}	// namespace graph

}	// namespace vl

#endif
