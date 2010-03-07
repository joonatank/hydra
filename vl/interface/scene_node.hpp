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

namespace vl
{

namespace graph
{
	// Forward declarations
	class SceneManager;
	class MovableObject;
	class SceneNode;

	typedef std::vector<SceneNode *> ChildContainer;
	typedef std::vector<MovableObject *> ObjectContainer;

	// Abstract scene node interface.
	// SceneNode is a basic element in scene graph and it is the only element
	// that can be other than leafs of the tree
	// (MovableObjects can be only be leafs).
	// It manages the attributes of the whole branch, like transformation
	// (combined, but nodes transformation is always applied).
	class SceneNode
	{
		public :
			// Instructs SceneManager to free this Node, all the ChildNodes
			// and all attached objects
			virtual void destroy( void ) = 0;

			//virtual uint32_t getID( void ) const = 0;
			// These methods just add dirtyBits to stored data,
			// they need to be overriden by the implementation which has
			// knowledge about the rendering engine types, unless of course
			// for the application (only the Nodes have concrete data).
			virtual void translate( vl::vector const &v ) = 0;

			virtual void setTranslation( vl::vector const &v ) = 0;

			virtual vl::vector const &getTranslation( void ) = 0;

			virtual void rotate( vl::quaternion const &q ) = 0;

			virtual void setRotation( vl::quaternion const &q ) = 0;

			virtual vl::quaternion const &getRotation( void ) = 0;

			virtual void scale( vl::vector const &s ) = 0;

			virtual void scale( vl::scalar const s ) = 0;

			virtual void setScale( vl::vector const &s ) = 0;

			virtual vl::vector const &getScale( void ) = 0;

			virtual void attachObject( MovableObject *object ) = 0;

			virtual void detachObject( MovableObject *object ) = 0;

			virtual SceneNode *createChild(
					std::string const &name = std::string() ) = 0;

			virtual void setParent( SceneNode *parent ) = 0;

			virtual void addChild( SceneNode *child ) = 0;

			virtual void removeChild( SceneNode *child ) = 0;

			virtual ChildContainer const &getChilds( void ) const = 0;

			virtual SceneNode *getParent( void ) = 0;

			virtual ObjectContainer const &getAttached( void ) const = 0;

			virtual SceneManager *getCreator( void ) = 0;

			// Overloadable functions to modify the Rendering engine
			// These have to be overriden but they are not pure abstracts
			// because this class can be used in the Application without
			// rendering engine.
			// TODO
			// Renaming them to callbacks might be usefull like
			// transformationSet, scaleSet, objectAttached etc.
			virtual void _setTransform( vl::vector const &,
					vl::quaternion const & ) = 0;

			virtual void _setScale( vl::vector const & ) = 0;

			virtual void _attachObject( MovableObject *) = 0;

			virtual void _detachObject( MovableObject *) = 0;

			virtual void _addChild( SceneNode *) = 0;

			virtual void _removeChild( SceneNode *) = 0;

	};	// class SceneNode

}	// namespace graph

}	// namespace vl

#endif
