#ifndef EQOGRE_FRAME_DATA_HPP
#define EQOGRE_FRAME_DATA_HPP

#include <eq/fabric/serializable.h>
#include <eq/client/config.h>
#include <co/dataIStream.h>
#include <co/dataOStream.h>

#include <OGRE/OgreVector3.h>
#include <OGRE/OgreQuaternion.h>

#include "scene_node.hpp"

namespace eqOgre
{

class SceneManager : public eq::fabric::Serializable
{
public :
	struct SceneNodeIDPair
	{
		SceneNodeIDPair( SceneNode *nnode = 0,
						 eq::base::UUID const &nid = eq::base::UUID::ZERO )
			: node(nnode), id(nid)
		{}

		SceneNode *node;
		eq::base::UUID id;
	};

	SceneManager( void );

	virtual ~SceneManager( void );

	bool setSceneManager( Ogre::SceneManager *man );

	/// Add a SceneNode to the distributed stack
	/// FrameData owns all SceneNodes added using this method and will destroy
	/// them when necessary.
	void addSceneNode( SceneNodePtr node );

	bool hasSceneNode( std::string const &name ) const;

	SceneNodePtr getSceneNode( std::string const &name );

	const SceneNodePtr getSceneNode( std::string const &name ) const;

	SceneNodePtr getSceneNode( size_t i );

	const SceneNodePtr getSceneNode( size_t i ) const;

	size_t getNSceneNodes( void ) const
	{ return _scene_nodes.size(); }

	// TODO add SceneNode removal

	void reloadScene( void );

	uint32_t getSceneVersion( void ) const
	{ return _scene_version; }

	eq::uint128_t commitAll( void );

	void syncAll( void );

	bool registerData( eq::Config *session );

	void deregisterData( void );

	void mapData( eq::Config *session, eq::base::UUID const &id );

	void unmapData( void );

	enum DirtyBits
	{
		DIRTY_NODES = eq::fabric::Serializable::DIRTY_CUSTOM << 0,
		DIRTY_RELOAD_SCENE = eq::fabric::Serializable::DIRTY_CUSTOM << 1,
		DIRTY_ACTIVE_CAMERA = eq::fabric::Serializable::DIRTY_CUSTOM << 2,
		DIRTY_CUSTOM = eq::fabric::Serializable::DIRTY_CUSTOM << 3
	};

protected :
	virtual void serialize( co::DataOStream &os, const uint64_t dirtyBits );
	virtual void deserialize( co::DataIStream &is, const uint64_t dirtyBits );

	void _mapObject( SceneNodeIDPair &node );

	bool _registerObject( SceneNodeIDPair &node );

private :
	std::vector< SceneNodeIDPair > _scene_nodes;

	// Reload the scene
	uint32_t _scene_version;

	// SceneManager used for creating mapping between eqOgre::SceneNode and
	// Ogre::SceneNode
	// Only valid on slaves and only needed when the SceneNode is mapped
	Ogre::SceneManager *_ogre_sm;

	eq::Config *_config;

};	// class FrameData

}	// namespace eqOgre

#endif
