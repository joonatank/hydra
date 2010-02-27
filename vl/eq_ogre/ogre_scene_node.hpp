#ifndef VL_OGRE_SCENE_NODE_HPP
#define VL_OGRE_SCENE_NODE_HPP

#include "eq_graph/eq_scene_node.hpp"

#include "eq_ogre/conversion.hpp"

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
			SceneNode( vl::ogre::SceneManager *creator,
					std::string const &name = std::string() );

			Ogre::SceneNode *getNative( void )
			{ return _ogre_node; }

			virtual ~SceneNode( void )
			{}

			virtual void _setTransform( vmml::vec3d const &vec,
					vmml::quaterniond const &q )
			{
				_ogre_node->setPosition( vl::math::convert( vec ) );
				_ogre_node->setOrientation( vl::math::convert(q) );
			}

			virtual void _setScale( vmml::vec3d const &s )
			{
				_ogre_node->setScale( vl::math::convert(s) );
			}

			virtual void _attachObject( vl::graph::MovableObject *obj )
			{
				MovableObject *ogre_obj = (MovableObject *)obj;
				_ogre_node->attachObject( ogre_obj->getNative() );
			}

			virtual void _detachObject( vl::graph::MovableObject *obj )
			{
				MovableObject *ogre_obj = (MovableObject *)obj;
				_ogre_node->detachObject( ogre_obj->getNative() );
			}

			virtual void _addChild( vl::graph::SceneNode *child)
			{
				SceneNode *ogre_child = (SceneNode *)child;
				_ogre_node->addChild( ogre_child->getNative() );
			}

			virtual void _removeChild( vl::graph::SceneNode *child )
			{
				SceneNode *ogre_child = (SceneNode *)child;
				_ogre_node->removeChild( ogre_child->getNative() );
			}

		protected :
			Ogre::SceneNode *_ogre_node;
	};
}	// namespace ogre

}	// namespace vl
#endif
