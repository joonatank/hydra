/*	Joonatan Kuosa
 *	2010-02
 *
 *	SceneGraph element: has transformation, can have parent, children and own
 *	objects.
 *
 *	This class should not depend on any external library.
 *	All conversions to external libraries should be done in separate class
 *	either through inheritance or aggregation.
 */

#ifndef VL_EQ_GRAPH_SCENE_NODE_HPP
#define VL_EQ_GRAPH_SCENE_NODE_HPP

#include <eq/client/object.h>

#include "interface/scene_node.hpp"

#include "eq_movable_object.hpp"
#include "base/exceptions.hpp"

//#include "eq_cluster/distrib_container.hpp"

namespace vl
{

namespace cl
{
	// Forward declarations
	class SceneManager;
	class SceneNode;

	// SceneNode is a basic element in scene graph and it is the only element
	// that can be other than leafs of the tree
	// (MovableObjects can be only be leafs).
	// It manages the attributes of the whole branch, like transformation
	// (combined, but nodes transformation is always applied).
	class SceneNode : public eq::Object, public vl::graph::SceneNode
	{
		public :
			SceneNode( vl::graph::SceneManager *creator,
					std::string const &name = std::string() );

			// Frees the memory, called from SceneManager
			virtual ~SceneNode( void ) {}

			virtual std::string const &getName( void )
			{ return eq::Object::getName(); }

			// These methods just add dirtyBits to stored data,
			// they need to be overriden by the implementation which has
			// knowledge about the rendering engine types, unless of course
			// for the application (only the Nodes have concrete data).
			virtual void translate( vl::vector const &v,
					TransformSpace relativeTo = TS_PARENT );

			virtual void setPosition( vl::vector const &v,
					TransformSpace relativeTo = TS_PARENT );

			virtual vl::vector const &getPosition( 
					TransformSpace relativeTo = TS_PARENT )
			{ return _position; }

			virtual void rotate( vl::quaternion const &q,
					TransformSpace relativeTo = TS_LOCAL );

			virtual void setOrientation( vl::quaternion const &q,
					TransformSpace relativeTo = TS_LOCAL );

			virtual vl::quaternion const &getOrientation(
					TransformSpace relativeTo = TS_LOCAL )
			{ return _rotation; }

			// TODO  implement
			virtual void lookAt( vl::vector const &v ) {}

			virtual void scale( vl::vector const &s );

			virtual void scale( vl::scalar const s );

			virtual void setScale( vl::vector const &s );

			virtual vl::vector const &getScale( void )
			{ return _scale; }

			virtual void attachObject( vl::graph::MovableObject *object );

			virtual void detachObject( vl::graph::MovableObject *object );

			virtual vl::graph::SceneNode *createChild(
					std::string const &name = std::string() );

			virtual void setParent( vl::graph::SceneNode *parent );

			virtual void addChild( vl::graph::SceneNode *child );

			virtual vl::graph::SceneNode *getChild( uint16_t index );

			virtual vl::graph::SceneNode *getChild( std::string const &name );

			virtual vl::graph::SceneNode *removeChild( uint16_t index );

			virtual vl::graph::SceneNode *removeChild( std::string const &name );

			virtual vl::graph::ChildContainer const &getChilds( void ) const
			{ return _childs; }

			virtual uint16_t numChildren( void )
			{ return _childs.size(); }

			virtual vl::graph::SceneNode *getParent( void )
			{ return _parent; }

			virtual vl::graph::ObjectContainer const &getAttached( void ) const
			{ return _attached; }

			virtual uint16_t numAttached( void )
			{ return _attached.size(); }

			virtual vl::graph::SceneManager *getCreator( void )
			{ return _creator; }

			// Sync/Commit methods, needed for tree based syncing in eq

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
				// Transform handles both translate and rotation
				DIRTY_TRANSFORM = eq::Object::DIRTY_CUSTOM << 0,
				// Scale should be rarely needed
				DIRTY_SCALE = eq::Object::DIRTY_CUSTOM << 1,
				// This handles the _attached container
				// This is only updated when the container created
				DIRTY_ATTACHED = eq::Object::DIRTY_CUSTOM << 2,
				// This handles the _child container
				// This is only updated when the container created
				DIRTY_CHILDS = eq::Object::DIRTY_CUSTOM << 3,
				// Some rarely needed attributes, might be better to
				// move them behind one variable.
				DIRTY_VISIBILITY = eq::Object::DIRTY_CUSTOM << 4,
				DIRTY_SHOW_BOUNDINGBOX = eq::Object::DIRTY_CUSTOM << 5,
				DIRTY_CUSTOM = eq::Object::DIRTY_CUSTOM << 6
			};

		protected :
			void _addChild( vl::graph::SceneNode *child );

			void _setParent( vl::graph::SceneNode *parent );

			void _removeChild( vl::graph::SceneNode *child );

			vl::cl::SceneNode *_findChild( std::string const &name );
			// Owner and creator of this node, we need this to create new
			// objects. As the scene manager handles ultimately creation and
			// destruction of all objects contained in it.
			vl::graph::SceneManager *_creator;

			vl::vector _position;
			vl::quaternion _rotation;
			vl::vector _scale;

			// For now we use one vector to manage all attached objects.
			// For large scenegraphs this will be problematic, because we need
			// to update the whole vector at once.
			// So we need to develop versioned container to replace this later.
			//DistributedContainer<vl::cl::MovableObject *> _attached;
			//std::vector<vl::cl::MovableObject *
			vl::graph::ObjectContainer _attached;

			// Parent node, we need this to inform our current parent
			// if we are moved to another.
			vl::graph::SceneNode *_parent;

			// The childs we own, if we are destroyed we will destroy these.
			//DistributedContainer<vl::cl::SceneNode *> _childs;
			vl::graph::ChildContainer _childs;

	};	// class SceneNode

}	// namespace eq

}	// namespace vl

#endif
