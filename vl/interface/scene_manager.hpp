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

#ifndef VL_GRAPH_SCENE_MANAGER_HPP
#define VL_GRAPH_SCENE_MANAGER_HPP

#include <string>
#include <stdint.h>

#include "scene_node.hpp"
#include "movable_object.hpp"
#include "entity.hpp"
#include "camera.hpp"

#include "base/typedefs.hpp"

namespace vl
{
	enum FOG_MODE
	{
		FOG_NONE,
		FOG_LINEAR,
		FOG_EXP,
		FOG_EXP2
	};
	
namespace graph
{
	// Abstract class for managing the scene,
	class SceneManager
	{
		public :

			virtual ~SceneManager( void ) {}

			virtual std::string const &getName( void ) const = 0;

			// Get the root of the scene graph, created implicitly
			virtual SceneNodeRefPtr getRootNode( void ) = 0;

			// Destroy the scene graph preserving the scene manager and
			// scene parameters.
			virtual void destroyGraph( void ) = 0;

			// Create a hanging SceneNode that is not attached to scene graph
			virtual SceneNodeRefPtr createNode(
					std::string const &name = std::string() ) = 0;

			virtual MovableObjectRefPtr createMovableObject(
					std::string const &typeName,
					std::string const &name,
					vl::NamedValuePairList const &params
						= vl::NamedValuePairList() ) = 0;
			
			virtual void setAmbientLight( vl::colour const &col ) = 0;

			virtual vl::colour const &getAmbientLight( void ) const = 0;

			virtual void setFog( vl::FOG_MODE mode, vl::colour const &colourDiffuse,
								 vl::scalar expDensity, vl::scalar linearStart,
								 vl::scalar linearEnd ) = 0;

			// Create an entity, not attached to scene graph.
			virtual EntityRefPtr createEntity(
					std::string const &name, std::string const &meshName ) = 0;

			// Create a camera
			virtual CameraRefPtr createCamera( std::string const & ) = 0;

			// Create a light
			virtual LightRefPtr createLight( std::string const & ) = 0;

			// Get scene node by name
			virtual SceneNodeRefPtr getNode( std::string const &name ) = 0;

			// Get scene node by id
			virtual SceneNodeRefPtr getNode( uint32_t id ) = 0;

			// Get movable object by id
			virtual MovableObjectRefPtr getObject( uint32_t id ) = 0;

			// Get camera
			virtual CameraRefPtr getCamera( std::string const &name ) = 0;

			// Get light
			virtual LightRefPtr getLight( std::string const &name ) = 0;

//			virtual void pushChildToAddedStack( uint32_t id,
//					ChildAddedFunctor const &handle ) = 0;

//			virtual void pushChildToRemovedStack( SceneNodeRefPtr child ) = 0;

			// Set the manager used to create SceneNodes
			// This call is only valid when there is no scene graph
			// i.e. no root node.
			// If scene graph is present it will throw.
			virtual void setSceneNodeFactory( SceneNodeFactoryPtr factory )= 0;

			virtual void addMovableObjectFactory(
					vl::graph::MovableObjectFactoryPtr factory,
					bool overwrite = false ) = 0;

			virtual void removeMovableObjectFactory(
					vl::graph::MovableObjectFactoryPtr factory ) = 0;

			virtual void removeMovableObjectFactory(
					std::string const &typeName ) = 0;

			virtual std::vector<std::string> movableObjectFactories( void ) = 0;

	};	// class SceneManager

}	// namespace graph

}	// namespace vl

#endif
