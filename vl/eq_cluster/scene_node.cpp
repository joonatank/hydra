
#include "scene_node.hpp"

eqOgre::SceneNodePtr
eqOgre::SceneNode::create(const std::string& name)
{
	return new SceneNode(name);
}

/// Public
eqOgre::SceneNode::SceneNode(const std::string& name)
	: Serializable(),
	  _name(name),
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
eqOgre::SceneNode::serialize(eq::net::DataOStream& os, const uint64_t dirtyBits)
{
    eq::fabric::Serializable::serialize(os, dirtyBits);

	if( dirtyBits & DIRTY_NAME )
	{
		os << _name;
	}
	// Serialize position
	if( dirtyBits & DIRTY_POSITION )
	{
		operator<<( _position, os );
	}
	// Serialize orientation
	if( dirtyBits & DIRTY_ORIENTATION )
	{
		operator<<( _orientation, os );
	}

	if( dirtyBits & DIRTY_VISIBILITY )
	{
		os << _visible;
	}

}

void
eqOgre::SceneNode::deserialize(eq::net::DataIStream& is, const uint64_t dirtyBits)
{
	eq::fabric::Serializable::deserialize(is, dirtyBits);

	// Deserialize name
	if( dirtyBits & DIRTY_NAME )
	{
		is >> _name;
	}
	// Deserialize position
	if( dirtyBits & DIRTY_POSITION )
	{
		operator>>( _position, is );

		// If we have a correct node we need to transform it
		if( _ogre_node )
		{ _ogre_node->setPosition(_position + _initial_position); }
	}
	// Deserialize orientation
	if( dirtyBits & DIRTY_ORIENTATION )
	{
		operator>>( _orientation, is );

		// If we have a correct node we need to transform it
		if( _ogre_node )
		{ _ogre_node->setOrientation(_orientation * _initial_orientation); }
	}

	if( dirtyBits & DIRTY_VISIBILITY )
	{
		is >> _visible;
		if( _ogre_node )
		{ _ogre_node->setVisible(_visible); }
	}
}

const std::string eqOgre::HideOperationFactory::TYPENAME = "HideOperation";

const std::string eqOgre::ShowOperationFactory::TYPENAME = "ShowOperation";
