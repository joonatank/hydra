/*	Joonatan Kuosa
 *	2010-02
 *
 *	Class for managing the application, this is basicly a Singleton
 *	the number of instances depends on the number of pipe threads we have.
 *	No enforcement for the Singleton is done.
 *
 *	Manages global settings, like paths and so on.
 *	Also manages the SceneManagers.
 *
 *	This class should not depend on any external library.
 *	All conversions to external libraries should be done in separate class
 *	either through inheritance or aggregation.
 */

#ifndef VL_GRAPH_ROOT_HPP
#define VL_GRAPH_ROOT_HPP

#include "base/typedefs.hpp"
#include "settings.hpp"

namespace vl
{
namespace graph
{
	class Root
	{
		public :
			virtual ~Root( void ) {}
			
			// Some initialization methods
			// Engine specific mostly, so they need to be overriden
			virtual void createRenderSystem( void ) = 0;

			virtual void init( void ) = 0;

			virtual vl::graph::RenderWindowRefPtr createWindow(
					std::string const &name, unsigned int w, unsigned int h,
					vl::NamedValuePairList const &
						= vl::NamedValuePairList() ) = 0;

			// For now we only allow one SceneManager to exists per
			// instance.
			virtual SceneManagerRefPtr getSceneManager(
					std::string const &name ) = 0;

			virtual SceneManagerRefPtr createSceneManager(
					std::string const &name ) = 0;

			virtual void destroySceneManager( std::string const &name ) = 0;

			virtual void setupResources( void ) = 0;

			virtual void loadResources(void) = 0;

	};	// class Root

}	// namespace graph

}	// namespace vl

#endif
