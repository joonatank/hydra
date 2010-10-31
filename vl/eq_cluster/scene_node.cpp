
#include "scene_node.hpp"

/// Public
eqOgre::SceneNode::SceneNode(const std::string& name)
	: Serializable(),
	  _name(name),
	  _position( Ogre::Vector3::ZERO ),
	  _orientation( Ogre::Quaternion::IDENTITY ),
	  _ogre_node(0)
{}

bool eqOgre::SceneNode::findNode(Ogre::SceneManager* man)
{
	if( man->hasSceneNode( _name ) )
	{
		_ogre_node = man->getSceneNode( _name );
		_ogre_node->setOrientation(_orientation);
		_ogre_node->setPosition(_position);
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
		{ _ogre_node->setPosition(_position); }
	}
	// Deserialize orientation
	if( dirtyBits & DIRTY_ORIENTATION )
	{
		operator>>( _orientation, is );

		// If we have a correct node we need to transform it
		if( _ogre_node )
		{ _ogre_node->setOrientation(_orientation); }
	}
}
