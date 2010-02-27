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

#include "interface/scene_manager.hpp"

#include "eq_scene_node.hpp"
#include "eq_entity.hpp"
//#include "eq_movable_object.hpp"
#include "eq_camera.hpp"

#include <eq/client/object.h>

namespace vl
{

namespace cl
{
	// Functor used by the master when new node is created
	// Rendering engine nodes have a similar but different Functor
	// which calls mapObject.
	// FIXME these can't work as the objects don't have IDs before
	// they are registered so we need to register them when they are
	// created.
	// We can still use similar functors for slaves though.
	class CreateNodeFunc : public SceneFunctor<vl::cl::SceneNode *>
	{
		public :
			// SceneManager of which scene graph this functor operates
			CreateNodeFunc( SceneManager *sm );

			virtual SceneNode *operator()( uint32_t const &id );

	};	// class CreateNodeFunc

	class DeleteNodeFunc : public SceneFunctor<vl::cl::SceneNode *>
	{
		public :
			// SceneManager of which scene graph this functor operates
			DeleteNodeFunc( vl::cl::SceneManager *sm );

			virtual SceneNode *operator()( uint32_t const &id );

	};	// class DeleteNodeFunc

	class CreateObjectFunc : public SceneFunctor<vl::cl::MovableObject *>
	{
		public :
			// SceneManager of which scene graph this functor operates
			CreateObjectFunc( vl::cl::SceneManager *sm );

			virtual MovableObject *operator()( uint32_t const &id );

	};	// class CreateObjectFunc

	class DeleteObjectFunc : public SceneFunctor<vl::cl::MovableObject *>
	{
		public :
			// SceneManager of which scene graph this functor operates
			DeleteObjectFunc( vl::cl::SceneManager *sm );

			virtual MovableObject *operator()( uint32_t const &id );

	};	// class DeleteObjectFunc

	// Abstract class for managing the scene,
	// might be concrete we need to see about that.
	//
	// Needs to be overriden by the used rendering engines scene manager
	//
	// For the application we only need the shared data without the rendering
	// engine implementation so this class might work as the concrete
	// implementation of application data structure.
	// Nodes still have to use the rendering engine specific class.
	class SceneManager : public eq::Object, public vl::graph::SceneManager
	{
		public :

			SceneManager( std::string const &name );

			virtual ~SceneManager( void )
			{}

			// TODO move the deallocators to protected impl code.
			virtual void destroy( vl::graph::SceneNode *node )
			{ delete node; }

			virtual void destroy( vl::graph::MovableObject *obj )
			{ delete obj; }

			virtual vl::graph::SceneNode *getRootNode( void )
			{
				if( !_root )
				{ _root = createNodeImpl( "Root" ); }
				return _root;
			}

			virtual vl::graph::SceneNode *createNode(
					std::string const &name = std::string() )
			{
				return getRootNode()->createChild( name );
			}

			virtual vl::graph::SceneNode *createNodeImpl(
					std::string const &name );

			virtual vl::graph::MovableObject* createEntity(
					std::string const &name, std::string const &meshName );

			virtual vl::graph::Camera *createCamera( std::string const & )
			{ return 0; }

			virtual vl::graph::SceneNode *getNode( std::string const &name );

			virtual vl::graph::SceneNode *getNode( uint32_t id );

			virtual vl::graph::MovableObject *getObject( uint32_t id );

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
				// Dirty for the object container
				DIRTY_OBJECTS = eq::Object::DIRTY_CUSTOM << 0,
				// Dirty for the node container
				DIRTY_NODES = eq::Object::DIRTY_CUSTOM << 1,
				DIRTY_ACTIVE_CAMERA = eq::Object::DIRTY_CUSTOM << 2,
				DIRTY_AMBIENT_LIGHT = eq::Object::DIRTY_CUSTOM << 3,
				DIRTY_CUSTOM = eq::Object::DIRTY_CUSTOM << 4
			};

		//	typedef std::map<uint32_t, SceneNode *> NodeMap;
		//	typedef std::map<uint32_t, MovableObject *> ObjectMap;
			void finalize( void );

		protected :
			virtual SceneNode *_createSceneNodeImpl( std::string const &name );

			virtual MovableObject* _createMovableObjectImpl(
					std::string const &typeName,
					std::string const &name,
					vl::NamedValuePairList const &params
						= vl::NamedValuePairList() );

			virtual void _createDistribContainers( void );

			// We'll use distributed container for created objects now.
			// Makes searching bit slower but we have easier time distributing
			// them. (and we can use functors to create and destroy objects).
			DistributedContainer<SceneNode *> *_nodes;
			DistributedContainer<MovableObject *> *_objects;

			CreateObjectFunc *_objectCreateFunc;
			DeleteObjectFunc *_objectDeleteFunc;

			CreateNodeFunc *_nodeCreateFunc;
			DeleteNodeFunc *_nodeDeleteFunc;

			vl::graph::SceneNode *_root;

	};	// class SceneManager

}	// namespace graph

}	// namespace vl

#endif
