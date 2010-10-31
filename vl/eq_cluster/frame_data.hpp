#ifndef EQOGRE_FRAME_DATA_HPP
#define EQOGRE_FRAME_DATA_HPP

#include <eq/fabric/serializable.h>
#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>

#include <OGRE/OgreVector3.h>
#include <OGRE/OgreQuaternion.h>

#include "scene_node.hpp"

namespace eqOgre
{

class FrameData : public eq::fabric::Serializable
{
public :
	FrameData( void )
		: _camera_pos( Ogre::Vector3::ZERO ), _camera_rotation( Ogre::Quaternion::IDENTITY ),
//		  _ogre_pos( Ogre::Vector3::ZERO ), _ogre_rotation( Ogre::Quaternion::IDENTITY ),
		  _ogre( "ogre" ),
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

	// Init Ogre Node
	bool findOgreNode( Ogre::SceneManager *man )
	{
		return _ogre.findNode(man);
	}

	SceneNode &getOgreNode( void )
	{ return _ogre; }

	SceneNode const &getOgreNode( void ) const
	{ return _ogre; }

	// Set Ogre parameters
/*
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
*/
	void setOgreID( uint64_t id )
	{
		_ogre_id = id;
	}

	uint64_t getOgreID( void ) const
	{
		return _ogre_id;
	}
	
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

	// Test scene node called Ogre
	SceneNode _ogre;
	uint64_t _ogre_id;

	// Reload the scene
	uint32_t _scene_version;

};	// class FrameData

}	// namespace eqOgre

#endif
