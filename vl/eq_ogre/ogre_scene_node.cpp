#include "ogre_scene_node.hpp"

#include "ogre_scene_manager.hpp"

vl::ogre::SceneNode::SceneNode(
		vl::ogre::SceneManager *creator, std::string const &name )
	: vl::cl::SceneNode( creator, name ), _ogre_node(0)
{
	// TODO create Ogre::SceneNode
	if( name.empty() )
	{ _ogre_node = creator->getNative()->createSceneNode(); }
	else
	{ _ogre_node = creator->getNative()->createSceneNode( name ); }
}

