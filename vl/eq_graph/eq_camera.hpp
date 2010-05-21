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
#ifndef VL_EQ_GRAPH_CAMERA_HPP
#define VL_EQ_GRAPH_CAMERA_HPP

#include "interface/camera.hpp"

#include "base/exceptions.hpp"

#include <string>

#include <eq/client/object.h>

namespace vl
{
namespace cl
{
	class Camera : public eq::Object, public vl::graph::Camera
	{
		public :
			Camera( std::string name, vl::NamedValuePairList const &params );

			virtual ~Camera( void ) {}

			virtual void setManager( vl::graph::SceneManagerRefPtr man );
			
			virtual std::string const &getTypename( void ) const;
			
			virtual void setProjectionMatrix( vmml::mat4d const &m )
			{}

			virtual void setViewMatrix( vl::matrix const &m ) {}

			virtual void setFarClipDistance( vl::scalar const &dist ) {}

			virtual void setNearClipDistance( vl::scalar const &dist ) {}
			
			virtual void setPosition( vl::vector const &pos )
			{}

		protected :
			vl::graph::SceneManagerWeakPtr _manager;
			std::string _name;

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
