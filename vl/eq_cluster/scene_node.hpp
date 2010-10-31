#ifndef EQ_OGRE_SCENE_NODE_HPP
#define EQ_OGRE_SCENE_NODE_HPP

#include <eq/fabric/serializable.h>

#include <OGRE/OgreVector3.h>
#include <OGRE/OgreQuaternion.h>
#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreSceneManager.h>

namespace eqOgre
{
	
class SceneNode : public eq::fabric::Serializable
{
public :
	SceneNode( std::string const &name );

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

	Ogre::Vector3 const &getOgrePosition( void ) const
	{ return _position; }

	void setOgrePosition( Ogre::Vector3 const &v )
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

	Ogre::SceneNode *_ogre_node;

};	// class SceneNode

inline
eq::net::DataOStream &operator<<( Ogre::Vector3 const &v, eq::net::DataOStream &os )
{
	os << v[0] << v[1] << v[2];
	return os;
}


inline
eq::net::DataIStream &operator>>( Ogre::Vector3 &v, eq::net::DataIStream &is )
{
	is >> v[0] >> v[1] >> v[2];
	return is;
}

inline
eq::net::DataOStream &operator<<( Ogre::Quaternion const &q, eq::net::DataOStream &os )
{
	os << q[0] << q[1] << q[2] << q[3];
	return os;
}


inline
eq::net::DataIStream &operator>>( Ogre::Quaternion &q, eq::net::DataIStream &is )
{
	is >> q[0] >> q[1] >> q[2] >> q[3];
	return is;
}

}	// namespace eqOgre

#endif // EQ_OGRE_SCENE_NODE_HPP