#include "ogre_scene_node.hpp"

#include "ogre_scene_manager.hpp"
#include "ogre_camera.hpp"
#include "ogre_entity.hpp"

vl::ogre::SceneNode::SceneNode(
		vl::ogre::SceneManager *creator, std::string const &name )
	: vl::cl::SceneNode( creator, name ), _ogre_node(0)
{
	if( name == "Root" )
	{ _ogre_node = creator->getNative()->getRootSceneNode(); }
	else if( name.empty() )
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
vl::ogre::SceneNode::lookAt( vl::vector const &v )
{
	vl::cl::SceneNode::lookAt(v);
	_ogre_node->lookAt( vl::math::convert(v), Ogre::Node::TS_WORLD );
}
void
vl::ogre::SceneNode::scale( vl::vector const &s )
{
	vl::cl::SceneNode::scale(s);
	_ogre_node->scale( vl::math::convert(s) );
}

void
vl::ogre::SceneNode::scale( vl::scalar const s )
{
	vl::cl::SceneNode::scale(s);
	_ogre_node->scale( s, s, s );
}

void
vl::ogre::SceneNode::setScale( vl::vector const &s )
{
	vl::cl::SceneNode::setScale(s);
	_ogre_node->setScale( vl::math::convert(s) );
}

void
vl::ogre::SceneNode::attachObject( vl::graph::MovableObject *object )
{
	vl::cl::SceneNode::attachObject( object );

	vl::ogre::MovableObject *og_mov =
		dynamic_cast<MovableObject *>(object);
	if( !og_mov )
	{
		throw vl::exception( "vl::ogre::SceneNode::attachObject",
				"no movable object" );
	}

	Ogre::MovableObject *ogre_object = og_mov->getNative();
	if( ogre_object )
	{ this->getNative()->attachObject( ogre_object ); }
	else
	{ throw vl::null_pointer("vl::ogre::SceneNode::attachObject"); }
}

void
vl::ogre::SceneNode::detachObject( vl::graph::MovableObject *object )
{
	vl::cl::SceneNode::detachObject( object );
	vl::ogre::MovableObject *og_mov =
		dynamic_cast<MovableObject *>(object);
	if( !og_mov )
	{
		throw vl::exception( "vl::ogre::SceneNode::attachObject",
				"no movable object" );
	}

	Ogre::MovableObject *ogre_object = og_mov->getNative();
	if( ogre_object )
	{ this->getNative()->detachObject( ogre_object ); }
	else
	{ throw vl::null_pointer("vl::ogre::SceneNode::detachObject"); }
}

void
vl::ogre::SceneNode::setParent( vl::graph::SceneNode *parent )
{
	vl::cl::SceneNode::setParent( parent );
	((SceneNode *)parent)->getNative()->addChild(this->getNative());
}

void
vl::ogre::SceneNode::addChild( vl::graph::SceneNode *child )
{
	vl::cl::SceneNode::addChild( child );
	this->getNative()->addChild( ((SceneNode *)child)->getNative() );
}

