#ifndef EQOGRE_FRAME_DATA_HPP
#define EQOGRE_FRAME_DATA_HPP

#include <eq/fabric/serializable.h>
#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>

#include <OgreVector3.h>
#include <OgreQuaternion.h>

namespace eqOgre
{

class FrameData : public eq::fabric::Serializable
{
public :
	FrameData( void )
		: _camera_pos( Ogre::Vector3::ZERO ), _camera_rotation( Ogre::Quaternion::IDENTITY ),
		  _ogre_pos( Ogre::Vector3::ZERO ), _ogre_rotation( Ogre::Quaternion::IDENTITY ),
		  _scene_version( 0 )
	{}

	~FrameData( void ) {}

	Ogre::Vector3 const &getCameraPosition( void ) const
	{ return _camera_pos; }

	void setCameraPosition( Ogre::Vector3 const &v )
	{ 
		setDirty( DIRTY_CAMERA );
		_camera_pos = v; 
	}
	
	Ogre::Quaternion const &getCameraRotation( void ) const
	{
		return _camera_rotation; 
	}

	void setCameraRotation( Ogre::Quaternion const &q )
	{
		setDirty( DIRTY_CAMERA );
		_camera_rotation = q; 
	}

	// Set Ogre parameters
	Ogre::Vector3 const &getOgrePosition( void ) const
	{ return _ogre_pos; }

	void setOgrePosition( Ogre::Vector3 const &v )
	{
		setDirty( DIRTY_OGRE );
		_ogre_pos = v; 
	}

	Ogre::Quaternion const &getOgreRotation( void ) const
	{ return _ogre_rotation; }

	void setOgreRotation( Ogre::Quaternion const &q )
	{
		setDirty( DIRTY_OGRE );
		_ogre_rotation = q; 
	}

	// TODO boolean value does not work that well
	void updateSceneVersion( void )
	{ 
		setDirty( DIRTY_RELOAD );
		_scene_version++;
	}

	uint32_t getSceneVersion( void ) const
	{
		return _scene_version;
	}
	
	enum DirtyBits
	{
		DIRTY_CAMERA = eq::fabric::Serializable::DIRTY_CUSTOM << 0,
		DIRTY_OGRE = eq::fabric::Serializable::DIRTY_CUSTOM << 1,
		DIRTY_RELOAD = eq::fabric::Serializable::DIRTY_CUSTOM << 2
	};

protected :
	virtual void serialize( eq::net::DataOStream &os, const uint64_t dirtyBits );
    virtual void deserialize( eq::net::DataIStream &is, const uint64_t dirtyBits );

private :
	// Camera parameters
	Ogre::Vector3 _camera_pos;
	Ogre::Quaternion _camera_rotation;

	// Ogre SceneNode parameters
	Ogre::Vector3 _ogre_pos;
	Ogre::Quaternion _ogre_rotation;

	// Reload the scene
	uint32_t _scene_version;

};	// class FrameData

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

#endif