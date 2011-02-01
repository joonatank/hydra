/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2011-01
 */

#ifndef EQ_OGRE_SCENE_MANAGER_HPP
#define EQ_OGRE_SCENE_MANAGER_HPP

#include <OGRE/OgreVector3.h>
#include <OGRE/OgreQuaternion.h>

#include "scene_node.hpp"
#include "distributed.hpp"
#include "session.hpp"

namespace eqOgre
{

class Config;

class SceneManager : public vl::Distributed
{
public :
	struct SceneNodeIDPair
	{
		SceneNodeIDPair( SceneNode *nnode = 0,
						 uint64_t nid = vl::ID_UNDEFINED )
			: node(nnode), id(nid)
		{}

		SceneNode *node;
		uint64_t id;
	};

	SceneManager( vl::Session *session );

	virtual ~SceneManager( void );

	bool setSceneManager( Ogre::SceneManager *man );

	SceneNodePtr createSceneNode( std::string const &name );

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

	enum DirtyBits
	{
		DIRTY_NODES = vl::Distributed::DIRTY_CUSTOM << 0,
		DIRTY_RELOAD_SCENE = vl::Distributed::DIRTY_CUSTOM << 1,
		DIRTY_CUSTOM = vl::Distributed::DIRTY_CUSTOM << 3
	};

	void finaliseSync( void );

protected :
	virtual void serialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits );
	virtual void deserialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits );

	void _mapObject( SceneNodeIDPair &node );

private :
	std::vector< SceneNodeIDPair > _scene_nodes;
	std::vector< SceneNodeIDPair > _new_scene_nodes;

	// Reload the scene
	uint32_t _scene_version;

	// SceneManager used for creating mapping between eqOgre::SceneNode and
	// Ogre::SceneNode
	// Only valid on slaves and only needed when the SceneNode is mapped
	Ogre::SceneManager *_ogre_sm;

	vl::Session *_session;

};	// class FrameData

}	// namespace eqOgre

#endif	// EQ_OGRE_SCENE_MANAGER_HPP
