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

namespace vl
{
namespace graph
{
	class RenderWindow;
	class SceneManager;

	class Root
	{
		
		public :
			Root( void ) {}

			virtual ~Root( void ) {}
			
			// Some initialization methods
			// Engine specific mostly, so they need to be overriden
			virtual void createRenderingSystem( void ) = 0;

			virtual void init( void ) = 0;

			virtual vl::graph::RenderWindow *createWindow(
					std::string const &, unsigned int ,
					unsigned int ,
					vl::NamedValuePairList const & ) = 0;

			// For now we only allow one SceneManager to exists per
			// instance.
			virtual SceneManager *getSceneManager(
					std::string const &name = std::string() ) = 0;

		protected :
			// Internal method for creating the SceneManagers
			// must be overloaded by the classes that inherit.
		//	virtual SceneManager *_createSceneManager(
		//			std::string const &name = std::string() ) = 0;

	};	// class Root

}	// namespace graph

}	// namespace vl

#endif
