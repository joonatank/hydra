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

// Base interface
#include "eq_graph/eq_camera.hpp"
#include "ogre_movable_object.hpp"

// Necessary types
#include "math/math.hpp"

// Ogre includes
#include <OGRE/OgreCamera.h>

namespace vl
{
namespace ogre 
{
	class Camera : public vl::cl::Camera, public vl::ogre::MovableObject
	{
		public :
			Camera( std::string name, vl::NamedValuePairList const &params );
			
			virtual ~Camera( void );

			virtual void setManager( vl::graph::SceneManagerRefPtr man );
			
			virtual Ogre::MovableObject *getNative( void );

			virtual void setProjectionMatrix( vl::matrix const &m );

			virtual void setViewMatrix( vl::matrix const &m );

			virtual void setFarClipDistance( vl::scalar const &dist );

			virtual void setNearClipDistance( vl::scalar const &dist );

			virtual void setPosition( vl::vector const &pos );
		protected :
			Ogre::Camera *_ogre_camera;

	};	// class Camera

	struct CameraFactory : public vl::graph::MovableObjectFactory
	{
		CameraFactory( void ) {}

		virtual ~CameraFactory( void ) {}

		virtual vl::graph::MovableObjectRefPtr create( std::string const &name,
				vl::NamedValuePairList const &params );
			
		virtual std::string const &typeName( void )
		{ return TYPENAME; }

		static const std::string TYPENAME;
	};

}	// namespace graph

}	// namespace vl

#endif
