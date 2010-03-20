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

namespace vl
{

namespace graph
{
	class MovableObject;
	class SceneNode;
	class Camera;
	class Entity;

	// Abstract class for managing the scene,
	class SceneManager
	{
		public :

			// Get the root of the scene graph, created implicitly
			virtual SceneNode *getRootNode( void ) = 0;

			// Create a hanging SceneNode that is not attached to scene graph
			virtual SceneNode *createNode(
					std::string const &name = std::string() ) = 0;

			// Create an entity, not attached to scene graph.
			virtual Entity* createEntity(
					std::string const &name, std::string const &meshName ) = 0;

			// Create a camera
			virtual Camera *createCamera( std::string const & ) = 0;

			// Get scene node by name
			virtual SceneNode *getNode( std::string const &name ) = 0;

			// Get scene node by id
			virtual SceneNode *getNode( uint32_t id ) = 0;

			// Get movable object by id
			virtual MovableObject *getObject( uint32_t id ) = 0;

	};	// class SceneManager

}	// namespace graph

}	// namespace vl

#endif
