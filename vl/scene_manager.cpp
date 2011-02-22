/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-01
 *	@file scene_manager.cpp
 */

#include "scene_manager.hpp"

/// Public
vl::SceneManager::SceneManager( vl::Session *session, uint64_t id )
	: _scene_version( 0 ), _ogre_sm(0), _session(session)
{
	assert( _session );
	_session->registerObject( this, OBJ_SCENE_MANAGER, id );
}

vl::SceneManager::~SceneManager( void )
{
	for( size_t i = 0; i < _scene_nodes.size(); ++i )
	{ delete _scene_nodes.at(i); }

	_scene_nodes.clear();
}

void
vl::SceneManager::setSceneManager( Ogre::SceneManager *man )
{
	assert( man );
	_ogre_sm = man;
}

vl::SceneNodePtr
vl::SceneManager::createSceneNode( std::string const &name, uint64_t id )
{
	SceneNodePtr node = new SceneNode( name, this );

	assert( !name.empty() || vl::ID_UNDEFINED != id );

	// Check that no two nodes have the same name
	if( !name.empty() && hasSceneNode(name) )
	{
		// TODO is this the right exception?
		BOOST_THROW_EXCEPTION( vl::duplicate() );
	}
	assert( _session );

	_session->registerObject( node, OBJ_SCENE_NODE, id );
	assert( node->getID() != vl::ID_UNDEFINED );
	_scene_nodes.push_back( node );

	return node;
}

bool
vl::SceneManager::hasSceneNode(const std::string& name) const
{
	return( getSceneNode(name) );
}

vl::SceneNodePtr
vl::SceneManager::getSceneNode(const std::string& name)
{
	for( size_t i = 0; i < _scene_nodes.size(); ++i )
	{
		SceneNode *node = _scene_nodes.at(i);
		if( node->getName() == name )
		{ return node; }
	}

	return 0;
}

const vl::SceneNodePtr
vl::SceneManager::getSceneNode(const std::string& name) const
{
	for( size_t i = 0; i < _scene_nodes.size(); ++i )
	{
		SceneNode *node = _scene_nodes.at(i);
		if( node->getName() == name )
		{ return node; }
	}

	return 0;
}

vl::SceneNode *
vl::SceneManager::getSceneNode(size_t i)
{
	return _scene_nodes.at(i);
}

const vl::SceneNodePtr
vl::SceneManager::getSceneNode(size_t i) const
{
	return _scene_nodes.at(i);
}

void vl::SceneManager::reloadScene( void )
{
	std::cerr << "Should reload the scene now." << std::endl;
	setDirty( DIRTY_RELOAD_SCENE );
	_scene_version++;
}


/// -------------------------------Protected -----------------------------------
void
vl::SceneManager::serialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits )
{
 	if( dirtyBits & DIRTY_RELOAD_SCENE )
 	{
 		msg << _scene_version;
 	}
}

void
vl::SceneManager::deserialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits )
{
	if( dirtyBits & DIRTY_RELOAD_SCENE )
	{
		msg >> _scene_version;
	}
}
