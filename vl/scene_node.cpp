/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2011-01
 */

#include "scene_node.hpp"

vl::SceneNodePtr
vl::SceneNode::create(const std::string& name)
{
	return new SceneNode(name);
}

/// Public
vl::SceneNode::SceneNode(const std::string& name)
	: _name(name),
	  _position( Ogre::Vector3::ZERO ),
	  _orientation( Ogre::Quaternion::IDENTITY ),
	  _visible(true),
	  _ogre_node(0)
{}

bool
vl::SceneNode::findNode(Ogre::SceneManager* man)
{
	if( _ogre_node )
	{ _ogre_node = 0; }

	if( man->hasSceneNode( _name ) )
	{
		_ogre_node = man->getSceneNode( _name );
		_ogre_node->setOrientation(_orientation );
		_ogre_node->setPosition(_position );
		_ogre_node->setVisible( _visible );

		return true;
	}
	else
	{ return false; }
}



/// Protected
// Does no transformation on the Ogre Node as the master copy should be in
// AppNode which does not have Ogre SceneGraph
void
vl::SceneNode::serialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits )
{
	if( dirtyBits & DIRTY_NAME )
	{
		msg << _name;
	}
	// Serialize position
	if( dirtyBits & DIRTY_POSITION )
	{
		msg << _position;
	}
	// Serialize orientation
	if( dirtyBits & DIRTY_ORIENTATION )
	{
		msg << _orientation;
	}

	if( dirtyBits & DIRTY_VISIBILITY )
	{
		msg << _visible;
	}
}

void
vl::SceneNode::deserialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits )
{
	// Deserialize name
	if( dirtyBits & DIRTY_NAME )
	{
		msg >> _name;
	}
	// Deserialize position
	if( dirtyBits & DIRTY_POSITION )
	{
		msg >> _position;

		// If we have a correct node we need to transform it
		if( _ogre_node )
		{ _ogre_node->setPosition(_position); }
	}
	// Deserialize orientation
	if( dirtyBits & DIRTY_ORIENTATION )
	{
		msg >> _orientation;

		// If we have a correct node we need to transform it
		if( _ogre_node )
		{ _ogre_node->setOrientation(_orientation); }
	}

	if( dirtyBits & DIRTY_VISIBILITY )
	{
		msg >> _visible;
		if( _ogre_node )
		{ _ogre_node->setVisible(_visible); }
	}
}


/// --------- Actions ----------
void
vl::HideAction::execute(void )
{
	if( !_node )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	_node->setVisibility( false );
}

void
vl::ShowAction::execute(void )
{
	if( !_node )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	_node->setVisibility( true );
}

void
vl::SetTransformation::execute(const vl::Transform& trans)
{
	if( !_node )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	_node->setPosition( trans.position );
	_node->setOrientation( trans.quaternion );
}

