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

#include <eq/client/object.h>

#include "scene_manager.hpp"
#include "render_window.hpp"

namespace vl
{
namespace graph
{
	class Root : public eq::Object
	{
		public :
			Root( void );

			virtual ~Root( void )
			{}
			
			// Some initialization methods
			// Engine specific mostly, so they need to be overriden
			virtual void createRenderingSystem( void ) {}

			virtual void init( void ) {}

			virtual vl::graph::RenderWindow *createWindow(
					std::string const &, unsigned int ,
					unsigned int ,
					vl::NamedValuePairList const & )
			{
				return 0;
			}

			// For now we only allow one SceneManager to exists per
			// instance.
			virtual SceneManager *getSceneManager(
					std::string const &name = std::string() );

			// Equalizer overrides
	
			/* Object::serialize() */
			virtual void serialize( eq::net::DataOStream& os,
									const uint64_t dirtyBits );

			/* Object::deserialize() */
			virtual void deserialize( eq::net::DataIStream& is,
									  const uint64_t dirtyBits );

			/* Object::ChangeType */
			virtual ChangeType getChangeType() const
			{ return DELTA; }

			enum DirtyBits
			{
				DIRTY_SCENE_MANAGER = eq::Object::DIRTY_CUSTOM << 0,
				DIRTY_SHUTDOWN = eq::Object::DIRTY_CUSTOM << 1,
				DIRTY_RESOURCE_LOCATION = eq::Object::DIRTY_CUSTOM << 2,
				DIRTY_ACTIVE_SCENE = eq::Object::DIRTY_CUSTOM << 3,
				DIRTY_CUSTOM = eq::Object::DIRTY_CUSTOM << 4
			};

		protected :
			// Internal method for creating the SceneManagers
			// must be overloaded by the classes that inherit.
			virtual SceneManager *_createSceneManager(
					std::string const &name = std::string() )
			{
				return new SceneManager( name );
			}

			// For now we only manage one scene manager.
			SceneManager *_scene_manager;
	};	// class Root

}	// namespace graph

}	// namespace vl

#endif
