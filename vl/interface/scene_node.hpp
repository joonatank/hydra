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

#ifndef VL_GRAPH_SCENE_NODE_HPP
#define VL_GRAPH_SCENE_NODE_HPP

#include "math/math.hpp"

#include <vector>

#include <memory>	// for std::auto_ptr
#include <boost/shared_ptr.hpp>		// for ref counted ptrs

#include "base/typedefs.hpp"

namespace vl
{

namespace graph
{
	// Forward declarations
	//class SceneManagerRefPtr;
	//class MovableObjectRefPtr;
	//class SceneNode;

	class ChildAddedFunctor
	{
	public :
		virtual ~ChildAddedFunctor( void ) {}

		virtual void operator()( vl::graph::SceneNode *child ) = 0;

		virtual void operator()( std::string const &name ) = 0;
	};

	// Abstract scene node interface.
	// SceneNode is a basic element in scene graph and it is the only element
	// that can be other than leafs of the tree
	// (MovableObjects can be only be leafs).
	// It manages the attributes of the whole branch, like transformation
	// (combined, but nodes transformation is always applied).
	class SceneNode
	{
		public :
			enum TransformSpace
			{
				TS_LOCAL = 0,
				TS_PARENT,
				TS_WORLD
			};

			virtual ~SceneNode( void ) {}

			virtual std::string getName( void ) = 0;

			// These methods just add dirtyBits to stored data,
			// they need to be overriden by the implementation which has
			// knowledge about the rendering engine types, unless of course
			// for the application (only the Nodes have concrete data).
			virtual void translate( vl::vector const &v,
					TransformSpace relativeTo = TS_PARENT ) = 0;

			virtual void setPosition( vl::vector const &v,
					TransformSpace relativeTo = TS_PARENT ) = 0;

			virtual vl::vector const &getPosition(
					TransformSpace relativeTo = TS_PARENT ) = 0;

			virtual void rotate( vl::quaternion const &q,
					TransformSpace relativeTo = TS_LOCAL ) = 0;

			virtual void setOrientation( vl::quaternion const &q,
					TransformSpace relativeTo = TS_LOCAL ) = 0;

			virtual vl::quaternion const &getOrientation(
					TransformSpace relativeTo = TS_LOCAL ) = 0;

			virtual void lookAt( vl::vector const &v ) = 0;

			virtual void scale( vl::vector const &s ) = 0;

			virtual void scale( vl::scalar const s ) = 0;

			virtual void setScale( vl::vector const &s ) = 0;

			virtual vl::vector const &getScale( void ) = 0;

			virtual void attachObject( MovableObjectRefPtr object ) = 0;

			virtual void detachObject( MovableObjectRefPtr object ) = 0;

			virtual ObjectContainer const &getAttached( void ) const = 0;

			virtual uint16_t numAttached( void ) = 0;

			virtual SceneNodeRefPtr createChild(
					std::string const &name = std::string() ) = 0;

			virtual void setParent( SceneNodeRefPtr parent ) = 0;

			virtual void addChild( SceneNodeRefPtr child ) = 0;

			virtual void removeChild( vl::graph::SceneNodeRefPtr child ) = 0;

			virtual SceneNodeRefPtr removeChild( uint16_t index ) = 0;

			virtual SceneNodeRefPtr removeChild( std::string const &name ) = 0;

			virtual SceneNodeRefPtr getChild( uint16_t index ) = 0;

			virtual SceneNodeRefPtr getChild( std::string const &name ) = 0;

			virtual ChildContainer const &getChilds( void ) const = 0;

			virtual uint16_t numChildren( void ) = 0;

			virtual SceneNodeRefPtr getParent( void ) = 0;

			virtual SceneManagerRefPtr getManager( void ) = 0;

	};	// class SceneNode

	// Factory class for creating SceneNodes
	class SceneNodeFactory
	{
		public :
			virtual ~SceneNodeFactory( void ) {}

			virtual SceneNodeRefPtr create(
					SceneManagerRefPtr manager, std::string const &name ) = 0;
	};

}	// namespace graph

}	// namespace vl

#endif
