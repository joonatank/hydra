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

#include "eq_graph/eq_camera.hpp"

#include "ogre_movable_object.hpp"

#include <OGRE/OgreCamera.h>

#include "math/conversion.hpp"

namespace vl
{
namespace ogre 
{
	class Camera : public vl::graph::Camera, public vl::ogre::MovableObject
	{
		public :
			Camera( Ogre::Camera *cam )
				: _ogre_camera(cam)
			{
				if( !_ogre_camera)
				{ throw vl::null_pointer( "vl::ogre::Camera::Camera" ); }
			}

			virtual ~Camera( void ) {}

			virtual Ogre::MovableObject *getNative( void )
			{
				std::cout << "vl::ogre::Camera::getNative = " << _ogre_camera
					<< std::endl;
				return _ogre_camera;
			}

			virtual void setProjectionMatrix( vl::matrix const &m )
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
