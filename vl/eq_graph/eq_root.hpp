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

#ifndef VL_EQ_GRAPH_ROOT_HPP
#define VL_EQ_GRAPH_ROOT_HPP

#include <eq/client/object.h>

#include "interface/root.hpp"

#include "eq_scene_manager.hpp"
#include "eq_render_window.hpp"

namespace vl
{
namespace cl
{
	class Root : public eq::Object, public vl::graph::Root
	{
		public :
			Root( vl::SettingsRefPtr settings );

			virtual ~Root( void )
			{}
			
			// Some initialization methods
			// Engine specific mostly, so they need to be overriden
			virtual void createRenderSystem( void ) {}

			virtual void init( void ) {}

			virtual vl::graph::RenderWindowRefPtr createWindow(
					std::string const &, unsigned int ,
					unsigned int ,
					vl::NamedValuePairList const &
						= vl::NamedValuePairList() )
			{ return vl::graph::RenderWindowRefPtr(); }

			// For now we only allow one SceneManager to exists per
			// instance.
			virtual vl::graph::SceneManagerRefPtr getSceneManager(
					std::string const &name );

			virtual vl::graph::SceneManagerRefPtr createSceneManager(
					std::string const &name );

			virtual void destroySceneManager( std::string const &name );

//			virtual std::string getDataDir( void )
//			{ return _base_dir + "/data/"; }

			virtual void setupResources( void ) {}

			virtual void loadResources(void) {}

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
			//
			// We use this to provide factory pattern, so that
			// createSceneManager need not be overloaded
			// (it also handles distribution)
			virtual vl::graph::SceneManagerRefPtr _createSceneManager(
					std::string const &name )
			{
				return vl::graph::SceneManagerRefPtr( new SceneManager( name ) );
			}

			std::vector<vl::graph::SceneManagerRefPtr> _scene_managers;

			vl::SettingsRefPtr _settings;
			// Data directory, contains config files for Ogre, equalizer and
			// our own.
			//std::string _base_dir;

	};	// class Root

}	// namespace graph

}	// namespace vl

#endif
