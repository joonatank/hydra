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
	
	FrameData( void );

	virtual ~FrameData( void );

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

	void setSceneManager( Ogre::SceneManager *man );

	/// Add a SceneNode to the distributed stack
	/// FrameData owns all SceneNodes added using this method and will destroy
	/// them when necessary.
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
		DIRY_HEAD = eq::fabric::Serializable::DIRTY_CUSTOM << 0,
		DIRTY_NODES = eq::fabric::Serializable::DIRTY_CUSTOM << 1,
		DIRTY_RELOAD_SCENE = eq::fabric::Serializable::DIRTY_CUSTOM << 2,
		// TODO the reset scene is not implemented at the moment
		// It should reset all distributed SceneNodes
		DIRTY_RESET_SCENE = eq::fabric::Serializable::DIRTY_CUSTOM << 3,
		DIRTY_CUSTOM = eq::fabric::Serializable::DIRTY_CUSTOM << 4
	};

protected :
	virtual void serialize( eq::net::DataOStream &os, const uint64_t dirtyBits );
	virtual void deserialize( eq::net::DataIStream &is, const uint64_t dirtyBits );

	void _mapObject( SceneNodeIDPair &node );

	void _registerObject( SceneNodeIDPair &node );

private :
	std::vector< SceneNodeIDPair > _scene_nodes;

	Ogre::Vector3 _head_pos;
	Ogre::Quaternion _head_orient;
	
	// Reload the scene
	uint32_t _scene_version;

	// Session where we are, this should be zero for slave nodes and non zero
	// for the master node.
	eq::Config *_config;

	// SceneManager used for creating mapping between eqOgre::SceneNode and
	// Ogre::SceneNode
	// Only valid on slaves and only needed when the SceneNode is mapped
	Ogre::SceneManager *_ogre_sm;
};	// class FrameData

}	// namespace eqOgre

#endif
