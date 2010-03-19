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
#ifndef VL_GRAPH_CAMERA_HPP
#define VL_GRAPH_CAMERA_HPP

namespace vl
{
namespace graph
{
	class Camera : public MovableObject
	{
		public :
			virtual void setProjectionMatrix( vl::matrix const &m ) = 0;

	};	// class Camera

}	// namespace graph

}	// namespace vl

#endif
