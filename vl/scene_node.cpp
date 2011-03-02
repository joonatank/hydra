/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-01
 *	@file scene_node.cpp
 */

#include "scene_node.hpp"

#include "scene_manager.hpp"

/// Public
vl::SceneNode::SceneNode( std::string const &name, vl::SceneManager *creator )
	: _name(name)
	, _position( Ogre::Vector3::ZERO )
	, _orientation( Ogre::Quaternion::IDENTITY )
	, _visible(true)
	, _show_boundingbox(false)
	, _ogre_node(0)
	, _creator(creator)
{
	assert( _creator );
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

	if( dirtyBits & DIRTY_BOUNDING_BOX )
	{
		msg << _show_boundingbox;
	}
}

void
vl::SceneNode::deserialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits )
{
	// Deserialize name
	if( dirtyBits & DIRTY_NAME )
	{
		msg >> _name;
		// name should never be empty
		// @todo add exception throwing
		assert( !_name.empty() );
		if( !_findNode() )
		{
			std::cout << "Ogre node = " << _name << " NOT found in the SG." << std::endl;
		}
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

	if( dirtyBits & DIRTY_BOUNDING_BOX )
	{
		msg >> _show_boundingbox;
		if( _ogre_node )
		{ _ogre_node->showBoundingBox(_show_boundingbox); }
	}
}

bool
vl::SceneNode::_findNode( void )
{
	if( _ogre_node )
	{ return true; }

	assert( _creator );
	assert( _creator->getNative() );

	if( _creator->getNative()->hasSceneNode( _name ) )
	{
		_ogre_node = _creator->getNative()->getSceneNode( _name );
		_ogre_node->setOrientation(_orientation);
		_ogre_node->setPosition(_position);
		_ogre_node->setVisible(_visible);

		return true;
	}
	else
	{ return false; }
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

