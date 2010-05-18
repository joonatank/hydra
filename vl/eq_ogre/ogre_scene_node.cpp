#include "ogre_scene_node.hpp"

#include "ogre_scene_manager.hpp"
#include "ogre_camera.hpp"
#include "ogre_entity.hpp"

vl::ogre::SceneNode::SceneNode( vl::graph::SceneManagerRefPtr creator,
		std::string const &name )
	: vl::cl::SceneNode( creator, name ), _ogre_node(0)
{
	boost::shared_ptr<SceneManager> man
		= boost::dynamic_pointer_cast<SceneManager>( creator );
	// TODO this should throw failed dynamic cast
	if( !man )
	{
		BOOST_THROW_EXCEPTION( vl::null_pointer() );
//		throw vl::null_pointer( "vl::ogre::SceneNode::translate" );
	}

	if( name == "Root" )
	{ _ogre_node = man->getNative()->getRootSceneNode(); }
	else if( name.empty() )
	{ _ogre_node = man->getNative()->createSceneNode(); }
	else
	{ _ogre_node = man->getNative()->createSceneNode( name ); }
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
	if( !_ogre_node )
	{
		BOOST_THROW_EXCEPTION( vl::null_pointer() );
//		throw vl::null_pointer( "vl::ogre::SceneNode::translate" );
	}
	_ogre_node->translate( vl::math::convert( v ) );
}

void
vl::ogre::SceneNode::setPosition( vl::vector const &v,
		TransformSpace relativeTo )
{
	vl::cl::SceneNode::setPosition(v, relativeTo);
	if( !_ogre_node )
	{
		BOOST_THROW_EXCEPTION( vl::null_pointer() );
//		throw vl::null_pointer( "vl::ogre::SceneNode::setPosition" );
	}
	_ogre_node->setPosition( vl::math::convert( v ) );
}

void
vl::ogre::SceneNode::rotate( vl::quaternion const &q,
		TransformSpace relativeTo )
{
	vl::cl::SceneNode::rotate(q, relativeTo);
	if( !_ogre_node )
	{
		BOOST_THROW_EXCEPTION( vl::null_pointer() );
//		throw vl::null_pointer( "vl::ogre::SceneNode::rotate" );
	}
	_ogre_node->rotate( vl::math::convert(q), Ogre::Node::TS_WORLD );
}

void
vl::ogre::SceneNode::setOrientation( vl::quaternion const &q,
		TransformSpace relativeTo )
{
	vl::cl::SceneNode::setOrientation(q, relativeTo);
	if( !_ogre_node )
	{
		BOOST_THROW_EXCEPTION( vl::null_pointer() );
	//	throw vl::null_pointer( "vl::ogre::SceneNode::setOrientation" );
	}
	_ogre_node->setOrientation( vl::math::convert(q) );
}

void
vl::ogre::SceneNode::lookAt( vl::vector const &v )
{
	vl::cl::SceneNode::lookAt(v);
	if( !_ogre_node )
	{
		BOOST_THROW_EXCEPTION( vl::null_pointer() );
	//	throw vl::null_pointer( "vl::ogre::SceneNode::lookAt" );
	}
	_ogre_node->lookAt( vl::math::convert(v), Ogre::Node::TS_WORLD );
}
void
vl::ogre::SceneNode::scale( vl::vector const &s )
{
	vl::cl::SceneNode::scale(s);
	if( !_ogre_node )
	{
		BOOST_THROW_EXCEPTION( vl::null_pointer() );
	//	throw vl::null_pointer( "vl::ogre::SceneNode::scale" );
	}
	_ogre_node->scale( vl::math::convert(s) );
}

void
vl::ogre::SceneNode::scale( vl::scalar const s )
{
	vl::cl::SceneNode::scale(s);
	if( !_ogre_node )
	{
		BOOST_THROW_EXCEPTION( vl::null_pointer() );
	//	throw vl::null_pointer( "vl::ogre::SceneNode::scale" );
	}
	_ogre_node->scale( s, s, s );
}

void
vl::ogre::SceneNode::setScale( vl::vector const &s )
{
	vl::cl::SceneNode::setScale(s);
	if( !_ogre_node )
	{
		BOOST_THROW_EXCEPTION( vl::null_pointer() );
	//	throw vl::null_pointer( "vl::ogre::SceneNode::setScale" );
	}
	_ogre_node->setScale( vl::math::convert(s) );
}

void
vl::ogre::SceneNode::attachObject( vl::graph::MovableObjectRefPtr object )
{
	vl::cl::SceneNode::attachObject( object );

//	char const *where = "vl::ogre::SceneNode::attachObject";
	
	boost::shared_ptr<vl::ogre::MovableObject> og_mov
		= boost::dynamic_pointer_cast<MovableObject>(object);
	if( !og_mov )
	{
		BOOST_THROW_EXCEPTION( vl::null_pointer() );
	//	throw vl::exception( where,
	//			"no movable object" );
	}

	Ogre::MovableObject *ogre_object = og_mov->getNative();
	if( ogre_object )
	{ this->getNative()->attachObject( ogre_object ); }
	else
	{
		std::string what("No ogre movable object of type : ");
		what.append( object->getTypename() );
	//	throw vl::exception(where, what.c_str());
		BOOST_THROW_EXCEPTION( vl::null_pointer() );
	}
}

void
vl::ogre::SceneNode::detachObject( vl::graph::MovableObjectRefPtr object )
{
	vl::cl::SceneNode::detachObject( object );

	boost::shared_ptr<vl::ogre::MovableObject> og_mov
		= boost::dynamic_pointer_cast<MovableObject>(object);
	if( !og_mov )
	{
		// TODO should be replaced with cast failed exception
		BOOST_THROW_EXCEPTION( vl::null_pointer() );
//		throw vl::exception( "vl::ogre::SceneNode::attachObject",
//				"no movable object" );
	}

	Ogre::MovableObject *ogre_object = og_mov->getNative();
	if( ogre_object )
	{ this->getNative()->detachObject( ogre_object ); }
	else
	{
		// TODO should be replaced with no native object execption
		BOOST_THROW_EXCEPTION( vl::null_pointer() );
	//	throw vl::null_pointer("vl::ogre::SceneNode::detachObject");
	}
}

void
vl::ogre::SceneNode::childAdded( vl::graph::SceneNodeRefPtr child )
{
	vl::cl::SceneNode::childAdded( child );

	boost::shared_ptr<SceneNode> og_child
		= boost::dynamic_pointer_cast<SceneNode>( child );
	if( !og_child )
	{
		BOOST_THROW_EXCEPTION( vl::null_pointer() );
//		throw vl::exception( "vl::ogre::SceneNode::attachObject",
//				"no scene node" );
	}

	this->getNative()->addChild( og_child->getNative() );
}

void
vl::ogre::SceneNode::childRemoved( vl::graph::SceneNodeRefPtr child )
{
	vl::cl::SceneNode::childRemoved( child );

	boost::shared_ptr<SceneNode> og_child
		= boost::dynamic_pointer_cast<SceneNode>( child );
	if( !og_child )
	{
		BOOST_THROW_EXCEPTION( vl::null_pointer() );
//		throw vl::exception( "vl::ogre::SceneNode::attachObject",
//				"no movable object" );
	}

	this->getNative()->removeChild( og_child->getNative() );
}

// ---------- SceneNodeFactory ----------
vl::graph::SceneNodeRefPtr
vl::ogre::SceneNodeFactory::create( vl::graph::SceneManagerRefPtr manager,
		std::string const &name )
{
	return vl::graph::SceneNodeRefPtr( new SceneNode( manager, name ) );
}

