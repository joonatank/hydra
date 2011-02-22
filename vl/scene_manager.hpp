/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-01
 *	@file scene_manager.hpp
 */

#ifndef VL_SCENE_MANAGER_HPP
#define VL_SCENE_MANAGER_HPP

#include <OGRE/OgreVector3.h>
#include <OGRE/OgreQuaternion.h>

#include "scene_node.hpp"
#include "distributed.hpp"
#include "session.hpp"

namespace vl
{

class SceneManager : public vl::Distributed
{
public :
	/// Constructor
	/// @param session a session this SceneManager belongs to,
	/// used for registering and mapping objects
	/// @param id undefined if this is to be registered 
	/// and valid if this is to be mapped
	SceneManager( vl::Session *session, uint64_t id = vl::ID_UNDEFINED );

	virtual ~SceneManager( void );

	void setSceneManager( Ogre::SceneManager *man );

	SceneNodePtr createSceneNode( std::string const &name, uint64_t id = vl::ID_UNDEFINED );

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

	Ogre::SceneManager *getNative( void )
	{ return _ogre_sm; }

	enum DirtyBits
	{
		DIRTY_RELOAD_SCENE = vl::Distributed::DIRTY_CUSTOM << 0,
		DIRTY_CUSTOM = vl::Distributed::DIRTY_CUSTOM << 1,
	};

protected :
	virtual void serialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits );
	virtual void deserialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits );

private :
	std::vector<SceneNode *> _scene_nodes;

	// Reload the scene
	uint32_t _scene_version;

	// SceneManager used for creating mapping between vl::SceneNode and
	// Ogre::SceneNode
	// Only valid on slaves and only needed when the SceneNode is mapped
	Ogre::SceneManager *_ogre_sm;

	vl::Session *_session;

};	// class FrameData

}	// namespace vl

#endif	// VL_SCENE_MANAGER_HPP
