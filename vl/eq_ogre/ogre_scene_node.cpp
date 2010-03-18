#include "ogre_scene_node.hpp"

#include "ogre_scene_manager.hpp"

vl::ogre::SceneNode::SceneNode(
		vl::ogre::SceneManager *creator, std::string const &name )
	: vl::cl::SceneNode( creator, name ), _ogre_node(0)
{
	if( name.empty() )
	{ _ogre_node = creator->getNative()->createSceneNode(); }
	else
	{ _ogre_node = creator->getNative()->createSceneNode( name ); }
}

vl::ogre::SceneNode::~SceneNode( void )
{
	// TODO delete Ogre::SceneNode
}

void
vl::ogre::SceneNode::translate( vl::vector const &v,
		TransformSpace relativeTo )
{
	vl::cl::SceneNode::translate(v, relativeTo);
	_ogre_node->translate( vl::math::convert( v ) );
}

void
vl::ogre::SceneNode::setPosition( vl::vector const &v,
		TransformSpace relativeTo )
{
	vl::cl::SceneNode::setPosition(v, relativeTo);
	_ogre_node->setPosition( vl::math::convert( v ) );
}

void
vl::ogre::SceneNode::rotate( vl::quaternion const &q,
		TransformSpace relativeTo )
{
	vl::cl::SceneNode::rotate(q, relativeTo);
	_ogre_node->rotate( vl::math::convert(q), Ogre::Node::TS_WORLD );
}

void
vl::ogre::SceneNode::setOrientation( vl::quaternion const &q,
		TransformSpace relativeTo )
{
	vl::cl::SceneNode::setOrientation(q, relativeTo);
	_ogre_node->setOrientation( vl::math::convert(q) );
}

void
vl::ogre::SceneNode::scale( vl::vector const &s )
{
	vl::cl::SceneNode::scale(s);
}

void
vl::ogre::SceneNode::scale( vl::scalar const s )
{
	vl::cl::SceneNode::scale(s);
}

void
vl::ogre::SceneNode::setScale( vl::vector const &s )
{
	vl::cl::SceneNode::setScale(s);
}

