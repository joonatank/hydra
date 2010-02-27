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

namespace vl
{
namespace cl
{
	class Camera : public vl::graph::Camera
	{
		public :
			Camera( void ) {}

			virtual ~Camera( void ) {}

			virtual void setProjectionMatrix( vmml::mat4d const &m ) = 0;

		protected :

	};	// class Camera

}	// namespace graph

}	// namespace vl

#endif
