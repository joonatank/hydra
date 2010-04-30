/*	Joonatan Kuosa
 *	2010-02
 *
 *	Manages one particular Scene.
 *	Owns all the objects in that scene and keeps them sync.
 *
 *	This class should not depend on any external library.
 *	All conversions to external libraries should be done in separate class
 *	either through inheritance or aggregation.
 */

#ifndef VL_EQ_GRAPH_SCENE_MANAGER_HPP
#define VL_EQ_GRAPH_SCENE_MANAGER_HPP

#include <eq/client/object.h>

#include <boost/enable_shared_from_this.hpp>

#include "interface/scene_manager.hpp"

#include "eq_scene_node.hpp"
#include "eq_entity.hpp"
#include "eq_camera.hpp"

namespace vl
{

namespace cl
{
	// Abstract class for managing the scene,
	// might be concrete we need to see about that.
	//
	// Needs to be overriden by the used rendering engines scene manager
	//
	// For the application we only need the shared data without the rendering
	// engine implementation so this class might work as the concrete
	// implementation of application data structure.
	// Nodes still have to use the rendering engine specific class.
	class SceneManager : public eq::Object, public vl::graph::SceneManager,
						 public boost::enable_shared_from_this<SceneManager>
	{
		public :

			SceneManager( std::string const &name );

			virtual ~SceneManager( void );

			virtual std::string const &getName( void ) const
			{ return _name; }

			virtual void destroyGraph( void );

			virtual vl::graph::SceneNodeRefPtr getRootNode( void );

			// TODO testing and implementation
			virtual void setAmbientLight( vl::colour const &col )
			{
			}

			// TODO testing and implementation
			virtual vl::colour const &getAmbientLight( void ) const
			{
				return vl::colour();
			}

			virtual vl::graph::SceneNodeRefPtr createNode(
					std::string const &name = std::string() );

			virtual vl::graph::MovableObjectRefPtr createMovableObject(
					std::string const &typeName,
					std::string const &name,
					vl::NamedValuePairList const &params
						= vl::NamedValuePairList() );

			virtual vl::graph::EntityRefPtr createEntity(
					std::string const &name, std::string const &meshName );

			// TODO testing and implementation
			virtual vl::graph::LightRefPtr createLight( std::string const & );

			// There is no implementation of vl::cl::Camera, because
			// it's render engine sepcifc and not distributed so this method
			// will always return NULL.
			virtual vl::graph::CameraRefPtr createCamera( std::string const & )
			{ return vl::graph::CameraRefPtr(); }

			virtual vl::graph::SceneNodeRefPtr getNode( std::string const &name );

			virtual vl::graph::SceneNodeRefPtr getNode( uint32_t id );

			virtual vl::graph::MovableObjectRefPtr getObject( uint32_t id );

			// Get camera
			// TODO testing and implementation
			virtual vl::graph::CameraRefPtr getCamera( std::string const &name )
			{
				return vl::graph::CameraRefPtr();
			}

			// Get light
			// TODO testing and implementation
			virtual vl::graph::LightRefPtr getLight( std::string const &name )
			{
				return vl::graph::LightRefPtr();
			}

			virtual void setSceneNodeFactory(
					vl::graph::SceneNodeFactoryPtr factory );

			virtual void addMovableObjectFactory(
					vl::graph::MovableObjectFactoryPtr factory,
					bool overwrite = false );

			virtual void removeMovableObjectFactory(
					vl::graph::MovableObjectFactoryPtr factory );

			virtual void removeMovableObjectFactory(
					std::string const &typeName );

			virtual std::vector<std::string> movableObjectFactories( void )
			{
				std::vector<std::string> names;
				std::map<std::string, vl::graph::MovableObjectFactoryPtr>::iterator iter;
				for( iter = _movable_factories.begin(); iter != _movable_factories.end();
						++iter )
				{
					names.push_back( iter->first );
				}
				return names;
			}

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
				// Dirty SceneGraph Root object
				DIRTY_ROOT = eq::Object::DIRTY_CUSTOM << 0,
				DIRTY_ACTIVE_CAMERA = eq::Object::DIRTY_CUSTOM << 1,
				DIRTY_AMBIENT_LIGHT = eq::Object::DIRTY_CUSTOM << 2,
				DIRTY_CUSTOM = eq::Object::DIRTY_CUSTOM << 3
			};

		protected :
			std::string _name;

			vl::graph::SceneNodeRefPtr _root;

			vl::graph::SceneNodeFactoryPtr _scene_node_factory;
			std::map<std::string, vl::graph::MovableObjectFactoryPtr>
				_movable_factories;

	};	// class SceneManager

}	// namespace graph

}	// namespace vl

#endif
