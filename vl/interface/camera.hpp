/*	Joonatan Kuosa
 *	2010-02
 *
 *	Camera class.
 *	Camera is responsible for rendering the scene.
 *	If we have multiple render viewports i.e. channels everyone of those
 *	channels will call setProjectionMatrix before rendering, so they can
 *	have their specific projection.
 *	This implies that when we have multiple Nodes working with one
 *	distributed camera all the settings are shared between local instances
 *	but the projection matrices differs.
 *
 *	Handles camera settings.
 *
 *	As with all the other scene objects is not movable despite the name.
 *	To move a camera it has to be attached to a scene node.
 *
 *  TODO
 *	Multiple cameras can exist in one scene and which of them are active 
 *	be changed through the scene manager.
 *	When using one channel per node configuration only one camera is necessary.
 *	For multiple channels, I think one camera per channel but this needs to
 *	be designed first.
 */
#ifndef VL_GRAPH_CAMERA_HPP
#define VL_GRAPH_CAMERA_HPP

#include "movable_object.hpp"

#include "math/math.hpp"

namespace vl
{
namespace graph
{
	class Camera : public MovableObject
	{
		public :
			virtual ~Camera( void ) {}

			virtual void setProjectionMatrix( vl::matrix const &m ) = 0;

			virtual void setViewMatrix( vl::matrix const &m ) = 0;

			virtual void setFarClipDistance( vl::scalar const &dist ) = 0;

			virtual void setNearClipDistance( vl::scalar const &dist ) = 0;

	};	// class Camera

}	// namespace graph

}	// namespace vl

#endif
