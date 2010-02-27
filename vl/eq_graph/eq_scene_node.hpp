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

#include <vmmlib/vector.hpp>
#include <vmmlib/quaternion.hpp>

#include "interface/scene_node.hpp"

#include "eq_movable_object.hpp"
#include "eq_cluster/distrib_container.hpp"

namespace vl
{

namespace cl
{
//	typedef std::vector<vl::graph::MovableObject *> MovableContainer;
//	typedef std::vector<vl::graph::SceneNode *> SceneNodeContainer;

	// Forward declarations
	class SceneManager;
	class SceneNode;

	// Functor declarations
	// Abstract functor for all conversion functors that use the SceneManager
	// (anything to do with the scene graph).
	// Also we have locked to input type to id now.
	template<typename T>
	class SceneFunctor : public ConversionFunctor<T, uint32_t>
	{
		public :
			SceneFunctor( vl::cl::SceneManager *sm )
				: _manager(sm)
			{}

			// Conversion function
			virtual T &operator()( uint32_t const &id ) = 0;

		protected :
			// SceneManager holds a list of all objects in the scene
			// and we can use it to convert IDs to object pointers
			vl::cl::SceneManager *_manager;
	};	// class SceneFunctor

	// Pointer specialization
	template<typename T>
	class SceneFunctor<T *> : public ConversionFunctor<T *, uint32_t>
	{
		public :
			SceneFunctor( vl::cl::SceneManager *sm )
				: _manager(sm)
			{}

			// Conversion function
			virtual T *operator()( uint32_t const &id ) = 0;

		protected :
			// SceneManager holds a list of all objects in the scene
			// and we can use it to convert IDs to object pointers
			vl::cl::SceneManager *_manager;
	};	// class SceneFunctor

	// Functor to which inherit conversion from id -> SceneNode
	// can also perform the necessary task of attaching node with id
	// to the parent (given when constructed).
	class AttachNodeFunc : public SceneFunctor<vl::cl::SceneNode *>
	{
		public :
			// SceneManager of which scene graph this functor operates
			// Owner node, that is the parent node for any attached nodes.
			// Use NULL owner to just convert id to node
			AttachNodeFunc( vl::cl::SceneManager *sm,
					vl::cl::SceneNode *owner = 0);

			virtual vl::cl::SceneNode *operator()( uint32_t const &id );

		protected :
			vl::cl::SceneNode *_owner;

	};	// class NodeModFunc

	class DetachNodeFunc : public SceneFunctor<vl::cl::SceneNode *>
	{
		public :
			// SceneManager of which scene graph this functor operates
			// Owner node, that is the parent node for any attached nodes.
			// Use NULL owner to just convert id to node
			DetachNodeFunc( vl::cl::SceneManager *sm,
					vl::cl::SceneNode *owner = 0);

			virtual vl::cl::SceneNode *operator()( uint32_t const &id );

		protected :
			vl::cl::SceneNode *_owner;

	};	// class NodeModFunc

	class AttachObjectFunc : public SceneFunctor<vl::cl::MovableObject *>
	{
		public :
			// SceneManager of which scene graph this functor operates
			// Owner node, that is the parent node for any attached nodes.
			// Use NULL owner to just convert id to node
			AttachObjectFunc( vl::cl::SceneManager *sm,
					vl::cl::SceneNode *owner = 0);

			virtual vl::cl::MovableObject *operator()( uint32_t const &id );

		protected :
			vl::cl::SceneNode *_owner;

	};	// class NodeModFunc

	class DetachObjectFunc : public SceneFunctor<vl::cl::MovableObject *>
	{
		public :
			// SceneManager of which scene graph this functor operates
			// Owner node, that is the parent node for any attached nodes.
			// Use NULL owner to just convert id to node
			DetachObjectFunc( vl::cl::SceneManager *sm,
					vl::cl::SceneNode *owner = 0);

			// Detaches this object from _owner
			virtual vl::cl::MovableObject *operator()( uint32_t const &id );

		protected :
			vl::cl::SceneNode *_owner;

	};	// class NodeModFunc

	// SceneNode is a basic element in scene graph and it is the only element
	// that can be other than leafs of the tree
	// (MovableObjects can be only be leafs).
	// It manages the attributes of the whole branch, like transformation
	// (combined, but nodes transformation is always applied).
	class SceneNode : public eq::Object, public vl::graph::SceneNode
	{
		public :
			SceneNode( vl::cl::SceneManager *creator,
					std::string const &name = std::string() );

			// Frees the memory, called from SceneManager
			virtual ~SceneNode( void ) {}

			//virtual uint32_t getID( void ) const
			//{ return eq::Object::getID(); }

			// Instructs SceneManager to free this Node, all the ChildNodes
			// and all attached objects
			virtual void destroy( void );

			// These methods just add dirtyBits to stored data,
			// they need to be overriden by the implementation which has
			// knowledge about the rendering engine types, unless of course
			// for the application (only the Nodes have concrete data).
			virtual void translate( vmml::vec3d const &v )
			{
				if( v != vmml::vec3d::ZERO )
				{
					setDirty( DIRTY_TRANSFORM );
					_position += v;
				}
			}

			virtual void setTranslate( vmml::vec3d const &v )
			{
				if( v != _position )
				{
					setDirty( DIRTY_TRANSFORM );
					_position = v;
				}
			}

			virtual void rotate( vmml::quaterniond const &q )
			{
				if( q == vmml::quaterniond::IDENTITY )
				{
					setDirty( DIRTY_TRANSFORM );
					_rotation *= q;
				}
			}

			virtual void setRotation( vmml::quaterniond const &q )
			{
				if( q != _rotation )
				{
					setDirty( DIRTY_TRANSFORM );
					_rotation = q;
				}
			}

			virtual void scale( vmml::vec3d const &s )
			{
				if( s != vmml::vec3d( 1, 1, 1 ) )
				{
					setDirty( DIRTY_SCALE );
					_scale *= s;
				}
			}

			virtual void scale( double const s )
			{
				if( s != 1 )
				{
					setDirty( DIRTY_SCALE );
					_scale *= s;
				}
			}

			virtual void setScale( vmml::vec3d const &s )
			{
				if( s != _scale )
				{
					setDirty( DIRTY_SCALE );
					_scale = s;
				}
			}

			virtual void attachObject( vl::graph::MovableObject *object );

			virtual void detachObject( vl::graph::MovableObject *object );

			virtual vl::graph::SceneNode *createChild(
					std::string const &name = std::string() );

			virtual void setParent( vl::graph::SceneNode *parent );

			virtual void addChild( vl::graph::SceneNode *child );

			virtual void removeChild( vl::graph::SceneNode *child )
			{
				vl::cl::SceneNode *cl_child = (vl::cl::SceneNode *)child;
				for( size_t i = 0; i < _childs.size(); ++i )
				{
					if( _childs.at(i) == cl_child )
					{
						_childs.remove(i);
						cl_child->setParent( 0 );
						break;
					}
				}
			}

			virtual vl::graph::SceneNode *parent( void )
			{ return _parent; }

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
				DIRTY_CUSTOM = eq::Object::DIRTY_CUSTOM << 6,
			};

			// Overloadable functions to modify the Rendering engine
			// These have to be overriden but they are not pure abstracts
			// because this class can be used in the Application without
			// rendering engine.
			virtual void _setTransform( vmml::vec3d const &,
					vmml::quaterniond const & )
			{}

			virtual void _setScale( vmml::vec3d const & ) {}

			virtual void _attachObject( vl::graph::MovableObject *) {} 

			virtual void _detachObject( vl::graph::MovableObject *) {}

			virtual void _addChild( vl::graph::SceneNode *) {}

			virtual void _removeChild( vl::graph::SceneNode *) {}

		protected :
			// Owner and creator of this node, we need this to create new
			// objects. As the scene manager handles ultimately creation and
			// destruction of all objects contained in it.
			vl::cl::SceneManager *_creator;

			vmml::vec3d _position;
			vmml::quaterniond _rotation;
			vmml::vec3d _scale;

			// Functors for distributed containers
			DetachNodeFunc _childDetachFunc;
			AttachNodeFunc _childAttachFunc;
			DetachObjectFunc _objectDetachFunc;
			AttachObjectFunc _objectAttachFunc;

			// For now we use one vector to manage all attached objects.
			// For large scenegraphs this will be problematic, because we need
			// to update the whole vector at once.
			// So we need to develop versioned container to replace this later.
			DistributedContainer<vl::cl::MovableObject *> _attached;

			// Parent node, we need this to inform our current parent
			// if we are moved to another.
			vl::graph::SceneNode *_parent;

			// The childs we own, if we are destroyed we will destroy these.
			DistributedContainer<vl::cl::SceneNode *> _childs;

	};	// class SceneNode

}	// namespace eq

}	// namespace vl

#endif
