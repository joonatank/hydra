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
				{ _root = createNode( "Root" ); }
				return _root;
			}

			virtual vl::graph::SceneNode *createNode(
					std::string const &name = std::string() );

			/*
			virtual vl::graph::SceneNode *createNodeImpl(
					std::string const &name );
			*/

			virtual vl::graph::MovableObject* createEntity(
					std::string const &name, std::string const &meshName );

			// There is no implementation of vl::cl::Camera, because
			// it's render engine sepcifc and not distributed so this method
			// will always return NULL.
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
				// Dirty SceneGraph Root object
				DIRTY_ROOT = eq::Object::DIRTY_CUSTOM << 0,
				DIRTY_ACTIVE_CAMERA = eq::Object::DIRTY_CUSTOM << 1,
				DIRTY_AMBIENT_LIGHT = eq::Object::DIRTY_CUSTOM << 2,
				DIRTY_CUSTOM = eq::Object::DIRTY_CUSTOM << 3
			};

		//	typedef std::map<uint32_t, SceneNode *> NodeMap;
		//	typedef std::map<uint32_t, MovableObject *> ObjectMap;

			// Nasty function to test some distribution stuff
			// remove when they are working.
			void finalize( void );

		protected :
			virtual SceneNode *_createSceneNodeImpl( std::string const &name );

			virtual MovableObject* _createMovableObjectImpl(
					std::string const &typeName,
					std::string const &name,
					vl::NamedValuePairList const &params
						= vl::NamedValuePairList() );

			vl::graph::SceneNode *_root;

	};	// class SceneManager

}	// namespace graph

}	// namespace vl

#endif
