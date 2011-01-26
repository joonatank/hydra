/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2011-01
 */

#include "scene_node.hpp"

eqOgre::SceneNodePtr
eqOgre::SceneNode::create(const std::string& name)
{
	return new SceneNode(name);
}

/// Public
eqOgre::SceneNode::SceneNode(const std::string& name)
	: _name(name),
	  _position( Ogre::Vector3::ZERO ),
	  _orientation( Ogre::Quaternion::IDENTITY ),
	  _initial_position( Ogre::Vector3::ZERO ),
	  _initial_orientation( Ogre::Quaternion::IDENTITY ),
	  _visible(true),
	  _ogre_node(0)
{}

bool eqOgre::SceneNode::findNode(Ogre::SceneManager* man)
{
	if( _ogre_node )
	{ _ogre_node = 0; }

	if( man->hasSceneNode( _name ) )
	{
		_ogre_node = man->getSceneNode( _name );
		_initial_orientation = _ogre_node->getOrientation();
		_initial_position = _ogre_node->getPosition();
		_ogre_node->setOrientation(_orientation * _initial_orientation);
		_ogre_node->setPosition(_position + _initial_position);
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
eqOgre::SceneNode::serialize( vl::cluster::Message &msg, const uint64_t dirtyBits )
{
	if( dirtyBits & DIRTY_NAME )
	{
		msg.write(_name);
	}
	// Serialize position
	if( dirtyBits & DIRTY_POSITION )
	{
		msg.write(_position);
	}
	// Serialize orientation
	if( dirtyBits & DIRTY_ORIENTATION )
	{
		msg.write(_orientation);
	}

	if( dirtyBits & DIRTY_VISIBILITY )
	{
		msg.write(_visible);
	}
}

void
eqOgre::SceneNode::deserialize( vl::cluster::Message &msg, const uint64_t dirtyBits )
{
	// Deserialize name
	if( dirtyBits & DIRTY_NAME )
	{
		msg.read(_name);
	}
	// Deserialize position
	if( dirtyBits & DIRTY_POSITION )
	{
		msg.read(_position);

		// If we have a correct node we need to transform it
		if( _ogre_node )
		{ _ogre_node->setPosition(_position + _initial_position); }
	}
	// Deserialize orientation
	if( dirtyBits & DIRTY_ORIENTATION )
	{
		msg.read(_orientation);

		// If we have a correct node we need to transform it
		if( _ogre_node )
		{ _ogre_node->setOrientation(_orientation * _initial_orientation); }
	}

	if( dirtyBits & DIRTY_VISIBILITY )
	{
		msg.read(_visible);
		if( _ogre_node )
		{ _ogre_node->setVisible(_visible); }
	}
}


/// --------- Actions ----------
void eqOgre::HideAction::execute(void )
{
	if( !_node )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	_node->setVisibility( false );
}

void eqOgre::ShowAction::execute(void )
{
	if( !_node )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	_node->setVisibility( true );
}

void eqOgre::SetTransformation::execute(const vl::Transform& trans)
{
	if( !_node )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	_node->setPosition( trans.position );
	_node->setOrientation( trans.quaternion );
}

