/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-01
 *	@file scene_manager.hpp
 */

#ifndef VL_SCENE_MANAGER_HPP
#define VL_SCENE_MANAGER_HPP

#include <OGRE/OgreVector3.h>
#include <OGRE/OgreQuaternion.h>
#include <OGRE/OgreColourValue.h>

// Necessary for PREFAB type
#include "entity.hpp"

#include "distributed.hpp"
#include "session.hpp"

#include "typedefs.hpp"

namespace vl
{

typedef std::vector<SceneNodePtr> SceneNodeList;
typedef std::vector<EntityPtr> EntityList;

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

	SceneNodePtr getSceneNode( std::string const &name ) const;

	SceneNodePtr getSceneNodeID(uint64_t id) const;

	size_t getNSceneNodes( void ) const
	{ return _scene_nodes.size(); }

	// TODO add SceneNode removal

	/// --- Entity ---
	EntityPtr createEntity( std::string const &name, vl::PREFAB type = PF_NONE, 
		uint64_t id = vl::ID_UNDEFINED );

	bool hasEntity( std::string const &name ) const;

	EntityPtr getEntity( std::string const &name ) const;

	EntityPtr getEntityID(uint64_t id) const;

	SceneNodeList const &getSceneNodeList(void) const
	{ return _scene_nodes; }

	EntityList const &getEntityList(void) const
	{ return _entities; }

	void reloadScene( void );

	uint32_t getSceneVersion( void ) const
	{ return _scene_version; }

	Ogre::SceneManager *getNative( void )
	{ return _ogre_sm; }

	void addToSelection( SceneNodePtr node );

	void removeFromSelection( SceneNodePtr node );

	bool isInSelection( SceneNodePtr node ) const;

	SceneNodeList const &getSelection( void ) const
	{ return _selection; }

	Ogre::ColourValue const &getAmbientLight(void) const
	{ return _ambient_light; }

	void setAmbientLight( Ogre::ColourValue const &colour )
	{
		if( _ambient_light != colour )
		{
			_ambient_light = colour;
			setDirty( DIRTY_AMBIENT_LIGHT );
		}
	}
	
	enum DirtyBits
	{
		DIRTY_RELOAD_SCENE = vl::Distributed::DIRTY_CUSTOM << 0,
		DIRTY_AMBIENT_LIGHT = vl::Distributed::DIRTY_CUSTOM << 1,
		DIRTY_CUSTOM = vl::Distributed::DIRTY_CUSTOM << 2,
	};

protected :
	virtual void serialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits );
	virtual void deserialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits );

private :
	SceneNodeList _scene_nodes;
	EntityList _entities;

	/// Selected SceneNodes
	/// @remarks
	/// At least for now not distributed
	/// the attributes for showing the selected nodes are distributed in the
	/// nodes them selves
	SceneNodeList _selection;

	// Reload the scene
	uint32_t _scene_version;

	Ogre::ColourValue _ambient_light;

	// SceneManager used for creating mapping between vl::SceneNode and
	// Ogre::SceneNode
	// Only valid on slaves and only needed when the SceneNode is mapped
	Ogre::SceneManager *_ogre_sm;

	vl::Session *_session;

};	// class FrameData

}	// namespace vl

#endif	// VL_SCENE_MANAGER_HPP
