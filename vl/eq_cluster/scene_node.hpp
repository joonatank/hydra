#ifndef EQ_OGRE_SCENE_NODE_HPP
#define EQ_OGRE_SCENE_NODE_HPP

#include <eq/fabric/serializable.h>

#include <OGRE/OgreVector3.h>
#include <OGRE/OgreQuaternion.h>
#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreSceneManager.h>

#include "serialize_helpers.hpp"

namespace eqOgre
{
	
class SceneNode : public eq::fabric::Serializable
{
public :
	SceneNode( std::string const &name = std::string() );

	~SceneNode( void )
	{}

	std::string const &getName( void ) const
	{
		return _name;
	}
	
	void setName( std::string const &name )
	{
		setDirty( DIRTY_NAME );
		_name = name;
	}

	/// Find the SceneNode with the same name from the Scene graph provided
	/// Returns true if found, false otherwise
	/// After this deserialization will transform the Ogre SceneNode
	bool findNode( Ogre::SceneManager *man );

	Ogre::Vector3 const &getPosition( void ) const
	{ return _position; }

	void setPosition( Ogre::Vector3 const &v )
	{
		setDirty( DIRTY_POSITION );
		_position = v;
	}

	Ogre::Quaternion const &getOrientation( void ) const
	{ return _orientation; }

	void setOrientation( Ogre::Quaternion const &q )
	{
		setDirty( DIRTY_ORIENTATION );
		_orientation = q;
	}
	
	enum DirtyBits
	{
		DIRTY_NAME = eq::fabric::Serializable::DIRTY_CUSTOM << 0,
		DIRTY_POSITION = eq::fabric::Serializable::DIRTY_CUSTOM << 1,
		DIRTY_ORIENTATION = eq::fabric::Serializable::DIRTY_CUSTOM << 2
	};

protected :
	virtual void serialize( eq::net::DataOStream &os, const uint64_t dirtyBits );
    virtual void deserialize( eq::net::DataIStream &is, const uint64_t dirtyBits );

private :
	std::string _name;
	
	Ogre::Vector3 _position;
	Ogre::Quaternion _orientation;

	// These are here for the time beign as the SceneGraph is only read on the
	// slave instances. The master does not have correct initial positions.
	// Can be removed when the master reads the dotscene file also.
	Ogre::Vector3 _initial_position;
	Ogre::Quaternion _initial_orientation;
	
	Ogre::SceneNode *_ogre_node;

};	// class SceneNode


}	// namespace eqOgre

#endif // EQ_OGRE_SCENE_NODE_HPP