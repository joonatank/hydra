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
#ifndef VL_GRAPH_VIEWPORT_HPP
#define VL_GRAPH_VIEWPORT_HPP

#include "math/math.hpp"

#include "base/typedefs.hpp"

namespace vl
{
namespace graph
{
	class Viewport 
	{
		public :
			virtual ~Viewport( void ) {}

			virtual void update( void ) = 0;

			virtual void clear( void ) = 0;

			virtual RenderWindowRefPtr getTarget( void ) = 0;

			virtual CameraRefPtr getCamera( void ) = 0;

			virtual void setCamera( CameraRefPtr cam ) = 0;

			virtual int getZOrder( void ) = 0;

			virtual void setBackgroundColour( vl::colour const &colour ) = 0;

	};	// class Viewport

}	// namespace graph

}	// namespace vl

#endif
