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

namespace vl
{
namespace graph
{
	class Viewport 
	{
		public :
			Viewport( void )
			{}

			virtual ~Viewport( void )
			{}

			virtual void setBackgroundColour(
					vmml::vector<4, double> const &colour ) = 0;

		protected :

	};

}	// namespace graph

}	// namespace vl

#endif
