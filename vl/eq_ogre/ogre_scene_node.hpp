#ifndef VL_OGRE_SCENE_NODE_HPP
#define VL_OGRE_SCENE_NODE_HPP

#include "eq_graph/eq_scene_node.hpp"

#include "math/conversion.hpp"

#include "eq_ogre/ogre_movable_object.hpp"

#include <OGRE/OgreSceneNode.h>

namespace vl
{
namespace ogre
{
	class SceneManager;
	// Concrete class that manages updating Ogre SceneNode from distributed
	// nodes.
	class SceneNode : public vl::cl::SceneNode
	{
		public :
			SceneNode( vl::graph::SceneManagerRefPtr creator,
					std::string const &name = std::string() );

			Ogre::SceneNode *getNative( void )
			{ return _ogre_node; }

			virtual ~SceneNode( void );

			virtual void translate( vl::vector const &v,
					TransformSpace relativeTo = TS_PARENT );

			virtual void setPosition( vl::vector const &v,
					TransformSpace relativeTo = TS_PARENT );

			virtual void rotate( vl::quaternion const &q,
					TransformSpace relativeTo = TS_LOCAL );

			virtual void setOrientation( vl::quaternion const &q,
					TransformSpace relativeTo = TS_LOCAL );

			virtual void lookAt( vl::vector const &v );

			virtual void scale( vl::vector const &s );

			virtual void scale( vl::scalar const s );

			virtual void setScale( vl::vector const &s );

			virtual void attachObject( vl::graph::MovableObjectRefPtr object );

			virtual void detachObject( vl::graph::MovableObjectRefPtr object );

			/*
			virtual void setParent( vl::graph::SceneNodeRefPtr parent );

			virtual void addChild( vl::graph::SceneNodeRefPtr child );
			*/

		protected :
			/* Override Callbacks
			 * 
			 */
			virtual void childAdded( vl::graph::SceneNodeRefPtr child );
			virtual void childRemoved( vl::graph::SceneNodeRefPtr child );

			Ogre::SceneNode *_ogre_node;

	};	// class SceneNode

	
	// Factory class for creating SceneNodes
	class SceneNodeFactory : public vl::graph::SceneNodeFactory
	{
		public :
			virtual ~SceneNodeFactory( void ) {}

			virtual vl::graph::SceneNodeRefPtr create(
					vl::graph::SceneManagerRefPtr manager, std::string const &name );
	};

}	// namespace ogre

}	// namespace vl

#endif
