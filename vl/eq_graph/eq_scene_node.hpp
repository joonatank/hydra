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
#include "interface/entity.hpp"
#include "interface/movable_object.hpp"

#include "eq_movable_object.hpp"
#include "base/exceptions.hpp"

#include "math/math.hpp"

#include <boost/enable_shared_from_this.hpp>

//#include "eq_cluster/distrib_container.hpp"

namespace vl
{

namespace cl
{
	// Forward declarations
	class SceneManager;
	class SceneNode;

	// Handle addition to syncronization stack functor
	class ChildAddedFunctor : public vl::graph::ChildAddedFunctor
	{
	public :
		ChildAddedFunctor( vl::graph::SceneNode *owner )
			: _owner( owner )
		{}

		virtual ~ChildAddedFunctor( void ) {}

		virtual void operator()( vl::graph::SceneNode *me )
		{
		}

	protected :
		// The new owner
		vl::graph::SceneNode *_owner;
	};

	// SceneNode is a basic element in scene graph and it is the only element
	// that can be other than leafs of the tree
	// (MovableObjects can be only be leafs).
	// It manages the attributes of the whole branch, like transformation
	// (combined, but nodes transformation is always applied).
	class SceneNode : public eq::Object, public vl::graph::SceneNode,
					  public boost::enable_shared_from_this<SceneNode>
	{
		public :
			SceneNode( vl::graph::SceneManagerRefPtr creator,
					std::string const &name = std::string() );

			// Frees the memory, called from SceneManager
			virtual ~SceneNode( void );

			virtual std::string const &getName( void ) const
			{ return _name; }

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
					TransformSpace relativeTo = TS_LOCAL ) const
			{ return _rotation; }

			// TODO  implement and test
			virtual void lookAt( vl::vector const &v ) {}

			// TODO  implement and test
			virtual void setDirection( vl::vector const &v ) {}

			virtual void scale( vl::vector const &s );

			virtual void scale( vl::scalar const s );

			virtual void setScale( vl::vector const &s );

			virtual vl::vector const &getScale( void ) const
			{ return _scale; }

			virtual void attachObject( vl::graph::MovableObjectRefPtr object );

			virtual void detachObject( vl::graph::MovableObjectRefPtr object );

			virtual vl::graph::SceneNodeRefPtr createChild(
					std::string const &name = std::string() );

			/* Set the parent SceneNode,
			 * Parent can be NULL pointer which will destroy this SceneNode if
			 * no other parent is set before the this ref ptr goes out of scope.
			 *
			 * Throws vl::duplicate if parent is this.
			 */
			virtual void setParent( vl::graph::SceneNodeRefPtr parent );

			virtual void addChild( vl::graph::SceneNodeRefPtr child );

			virtual void removeChild( vl::graph::SceneNodeRefPtr child );

			virtual vl::graph::SceneNodeRefPtr removeChild( uint16_t index );

			virtual vl::graph::SceneNodeRefPtr removeChild( std::string const &name );

			virtual vl::graph::SceneNodeRefPtr getChild( uint16_t index ) const;

			virtual vl::graph::SceneNodeRefPtr getChild( std::string const &name ) const;

			virtual vl::graph::ChildContainer const &getChilds( void ) const
			{ return _childs; }

			virtual uint16_t numChildren( void ) const
			{ return _childs.size(); }

			virtual vl::graph::SceneNodeRefPtr getParent( void ) const
			{ return _parent.lock(); }

			virtual vl::graph::ObjectContainer const &getAttached( void ) const
			{ return _attached; }

			virtual uint16_t numAttached( void ) const
			{ return _attached.size(); }

			virtual vl::graph::SceneManagerRefPtr getManager( void ) const
			{ return _manager.lock(); }

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
			// Callbacks
			// These handle both the setting dirty bits and also can be
			// overloaded by Rendering Backend to implement similar functionality.
			virtual void childAdded( vl::graph::SceneNodeRefPtr child );
			virtual void childRemoved( vl::graph::SceneNodeRefPtr child );

			/*
			void _addChild( vl::graph::SceneNode *child );

			void _setParent( vl::graph::SceneNode *parent );

			void _removeChild( vl::graph::SceneNode *child );
			*/

			vl::graph::SceneNodeRefPtr _findChild( std::string const &name ) const;

			vl::graph::ChildContainer::iterator _findChildIter( std::string const &name );

			//vl::graph::ChildContainer::iterator _findChildIter( vl::graph::

			// Owner and creator of this node, we need this to create new
			// objects. As the scene manager handles ultimately creation and
			// destruction of all objects contained in it.
			vl::graph::SceneManagerWeakPtr _manager;

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
			vl::graph::SceneNodeWeakPtr _parent;

			// The childs we own, if we are destroyed we will destroy these.
			//DistributedContainer<vl::cl::SceneNode *> _childs;
			vl::graph::ChildContainer _childs;

			std::string _name;

	};	// class SceneNode

	class DefaultSceneNodeFactory : public vl::graph::SceneNodeFactory
	{
		public :
			DefaultSceneNodeFactory( void ) {}

			virtual ~DefaultSceneNodeFactory( void ) {}

			virtual vl::graph::SceneNodeRefPtr create(
					vl::graph::SceneManagerRefPtr manager, std::string const &name )
			{
				return vl::graph::SceneNodeRefPtr( new SceneNode( manager, name ) );
			}

	};	// class DefaultSceneNodeFactory

// Finding
template< typename T >
boost::shared_ptr<T> find( vl::graph::SceneNodeRefPtr node, std::string const &name, std::string const &typeName );

}	// namespace eq

}	// namespace vl


// Finding
// Simple tree find algorithm
template< typename T >
boost::shared_ptr<T>
vl::cl::find( vl::graph::SceneNodeRefPtr node, std::string const &name, std::string const &typeName )
{
	/*
	// go through the attached objects
	if( typeName != "SceneNode" )
	{
		for( int i = 0; _attached.size(); i++ )
		{
			vl::graph::MovableObjectRefPtr obj = _attached.at(i);
			if( obj->typeName() == typeName && obj->getName() == name )
			{ return obj; }
		}
	}
	else
	{
		if( getName() == name )
		{ return shared_from_this(); }
	}
	
	for( int i = 0; i < _childs.size(); i++ )
	{
		return 
	}
	*/
}

#endif
