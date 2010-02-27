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

			// TODO move the deallocators to protected impl code.
			virtual void destroy( SceneNode *node ) = 0;

			virtual void destroy( MovableObject *obj ) = 0;

			virtual SceneNode *getRootNode( void ) = 0;

			virtual SceneNode *createNode(
					std::string const &name = std::string() ) = 0;

			virtual SceneNode *createNodeImpl( std::string const &name ) = 0;

			virtual MovableObject* createEntity(
					std::string const &name, std::string const &meshName ) = 0;

			virtual Camera *createCamera( std::string const & ) = 0;

			virtual SceneNode *getNode( std::string const &name ) = 0;

			virtual SceneNode *getNode( uint32_t id ) = 0;

			virtual MovableObject *getObject( uint32_t id ) = 0;

			virtual void finalize( void ) = 0;

		protected :
			/*
			virtual SceneNode *_createSceneNodeImpl( std::string const &name );

			virtual MovableObject* _createMovableObjectImpl(
					std::string const &typeName,
					std::string const &name,
					vl::NamedValuePairList const &params
						= vl::NamedValuePairList() );

			virtual void _createDistribContainers( void );
			*/

	};	// class SceneManager

}	// namespace graph

}	// namespace vl

#endif
