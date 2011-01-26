#ifndef EQOGRE_FRAME_DATA_HPP
#define EQOGRE_FRAME_DATA_HPP

#include <eq/fabric/serializable.h>
#include <eq/client/config.h>
#include <co/dataIStream.h>
#include <co/dataOStream.h>

#include <OGRE/OgreVector3.h>
#include <OGRE/OgreQuaternion.h>

#include "scene_node.hpp"
#include "distributed.hpp"

namespace eqOgre
{

class Config;

class SceneManager : public vl::Distributed
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

// 	void commit( void );
//
// 	void sync( void );

	bool registerData( eqOgre::Config *session );

	void deregisterData( void );

// 	bool mapData( eq::Config *session, uint64_t const id );

// 	void unmapData( void );

	enum DirtyBits
	{
		DIRTY_NODES = vl::Distributed::DIRTY_CUSTOM << 0,
		DIRTY_RELOAD_SCENE = vl::Distributed::DIRTY_CUSTOM << 1,
		DIRTY_ACTIVE_CAMERA = vl::Distributed::DIRTY_CUSTOM << 2,
		DIRTY_CUSTOM = vl::Distributed::DIRTY_CUSTOM << 3
	};

protected :
	virtual void serialize( vl::cluster::Message &msg, const uint64_t dirtyBits );
	virtual void deserialize( vl::cluster::Message &msg, const uint64_t dirtyBits );

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

	eqOgre::Config *_config;

};	// class FrameData

}	// namespace eqOgre

#endif
