#ifndef EQOGRE_FRAME_DATA_HPP
#define EQOGRE_FRAME_DATA_HPP

#include <eq/fabric/serializable.h>
#include <eq/client/config.h>
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
	struct SceneNodeIDPair
	{
		SceneNodeIDPair( SceneNode *nnode = 0, uint32_t nid = EQ_ID_INVALID )
			: node(nnode), id(nid)
		{}

		SceneNode *node;
		uint32_t id;
	};
	
	FrameData( void )
		: _scene_version( 0 )
	{}

	~FrameData( void ) {}

	/// Tracking related
	/// Will be removed soon
	void setHeadPosition( Ogre::Vector3 const &pos )
	{
		setDirty( DIRY_HEAD );
		_head_pos = pos;
	}

	Ogre::Vector3 const &getHeadPosition( void ) const
	{
		return _head_pos;
	}

	void setHeadOrientation( Ogre::Quaternion const &quat )
	{
		setDirty( DIRY_HEAD );
		_head_orient = quat;
	}

	Ogre::Quaternion const &getHeadOrientation( void ) const
	{
		return _head_orient;
	}

	bool findNodes( Ogre::SceneManager *man );

	/// New dynamic SceneNode interface
	void addSceneNode( SceneNode * node );

	SceneNode *getSceneNode( std::string const &name );

	SceneNode *getSceneNode( size_t i );

	size_t getNSceneNodes( void ) const
	{ return _scene_nodes.size(); }

	// TODO add SceneNode removal

	void updateSceneVersion( void )
	{ 
		setDirty( DIRTY_RELOAD_SCENE );
		_scene_version++;
	}

	uint32_t getSceneVersion( void ) const
	{
		return _scene_version;
	}

	uint32_t commitAll( void );

	void syncAll( void );

	void registerData( eq::Config *config );

	void deregisterData( eq::Config *config );

	void mapData( eq::Config *config, uint32_t id );

	void unmapData( eq::Config *config );
	
	enum DirtyBits
	{
		DIRY_HEAD = eq::fabric::Serializable::DIRTY_CUSTOM << 1,
		DIRTY_NODES = eq::fabric::Serializable::DIRTY_CUSTOM << 2,
//		DIRTY_OGRE = eq::fabric::Serializable::DIRTY_CUSTOM << 3,
		DIRTY_RELOAD_SCENE = eq::fabric::Serializable::DIRTY_CUSTOM << 4,
		// TODO the reset scene is not implemented at the moment
		// It should reset all distributed SceneNodes
		DIRTY_RESET_SCENE = eq::fabric::Serializable::DIRTY_CUSTOM << 5,
		DIRTY_CUSTOM = eq::fabric::Serializable::DIRTY_CUSTOM << 6
	};

protected :
	virtual void serialize( eq::net::DataOStream &os, const uint64_t dirtyBits );
    virtual void deserialize( eq::net::DataIStream &is, const uint64_t dirtyBits );

private :
	// TODO Also it's necessary to
	// refactor the sync, commit, register, deregister, map, unmap functions
	// TODO dynamically adding more nodes
	std::vector< SceneNodeIDPair > _scene_nodes;

	Ogre::Vector3 _head_pos;
	Ogre::Quaternion _head_orient;
	
	// Reload the scene
	uint32_t _scene_version;

};	// class FrameData

}	// namespace eqOgre

#endif
