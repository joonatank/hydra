/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2011-01
 */

#include "scene_manager.hpp"

#include "eq_cluster/config.hpp"

/// Public
eqOgre::SceneManager::SceneManager( vl::Session *session )
	: _scene_version( 0 ), _ogre_sm(0), _session(session)
{}

eqOgre::SceneManager::~SceneManager( void )
{
	for( size_t i = 0; i < _scene_nodes.size(); ++i )
	{ delete _scene_nodes.at(i).node; }

	_scene_nodes.clear();
}

bool
eqOgre::SceneManager::setSceneManager( Ogre::SceneManager *man )
{
	if( !man )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	_ogre_sm = man;

	bool retval = true;
	for( size_t i = 0; i < _scene_nodes.size(); ++i )
	{
		SceneNode *node = _scene_nodes.at(i).node;
		if( node )
		{
			if( !node->findNode(man) )
			{
				EQERROR << "No Ogre SceneNode with name " << node->getName()
					<< " found in the SceneGraph." << std::endl;
				retval = false;
			}
		}
	}

	return retval;
}

void
eqOgre::SceneManager::addSceneNode( eqOgre::SceneNode *node )
{
	EQASSERT( _session )
	setDirty( DIRTY_NODES );

	_session->registerObjectC( node );
	EQASSERT( node->getID() != vl::ID_UNDEFINED );
	_scene_nodes.push_back( SceneNodeIDPair(node, node->getID()) );

	EQINFO << "SceneNode : " << _scene_nodes.back().node->getName()
		<< " registered." << std::endl;
}

bool
eqOgre::SceneManager::hasSceneNode(const std::string& name) const
{
	return( getSceneNode(name) );
}

eqOgre::SceneNodePtr
eqOgre::SceneManager::getSceneNode(const std::string& name)
{
	for( size_t i = 0; i < _scene_nodes.size(); ++i )
	{
		SceneNode *node = _scene_nodes.at(i).node;
		if( node->getName() == name )
		{ return node; }
	}

	return 0;
}

const eqOgre::SceneNodePtr
eqOgre::SceneManager::getSceneNode(const std::string& name) const
{
	for( size_t i = 0; i < _scene_nodes.size(); ++i )
	{
		SceneNode *node = _scene_nodes.at(i).node;
		if( node->getName() == name )
		{ return node; }
	}

	return 0;
}

eqOgre::SceneNode *
eqOgre::SceneManager::getSceneNode(size_t i)
{
	return _scene_nodes.at(i).node;
}

const eqOgre::SceneNodePtr
eqOgre::SceneManager::getSceneNode(size_t i) const
{
	return _scene_nodes.at(i).node;
}

void eqOgre::SceneManager::reloadScene(void )
{
	std::cerr << "Should reload the scene now." << std::endl;
	setDirty( DIRTY_RELOAD_SCENE );
	_scene_version++;
}

void
eqOgre::SceneManager::finaliseSync( void )
{
	// Map all nodes that are missing Ogre SceneNode
	std::vector< std::vector<SceneNodeIDPair>::iterator > rem_cont;
	if( _ogre_sm )
	{
		std::vector<SceneNodeIDPair>::iterator iter;
		for( iter = _new_scene_nodes.begin(); iter != _new_scene_nodes.end(); ++iter )
		{
			if( !iter->node->findNode( _ogre_sm ) )
			{
				std::cout << "NO Ogre node = " << iter->node->getName()
					<< " found in the scene." << std::endl;
			}
			else
			{
				std::cout << "Ogre node = " << iter->node->getName()
					<< " found in the scene." << std::endl;
				rem_cont.push_back(iter);
			}
		}
	}

	for( size_t i = 0; i < rem_cont.size(); ++i )
	{
		_new_scene_nodes.erase( rem_cont.at(i) );
	}

	if( _new_scene_nodes.size() != 0 )
	{
		std::cout << "Still missing " << _new_scene_nodes.size()
			<< " Ogre SceneNodes." << std::endl;
	}
}


/// -------------------------------Protected -----------------------------------
// NOTE No registering can be done in the serialize method, it's called from
// different thread.
// FIXME  serialize is called from getInstanceData (DIRTY_ALL) when new version
// has been commited, why?
void
eqOgre::SceneManager::serialize( vl::cluster::Message &msg, const uint64_t dirtyBits )
{
	if( dirtyBits & DIRTY_NODES )
	{
		msg.write( _scene_nodes.size() );
		for( size_t i = 0; i < _scene_nodes.size(); ++i )
		{
			uint64_t id = _scene_nodes.at(i).id;
			std::string name = _scene_nodes.at(i).node->getName();
			EQASSERTINFO( id != vl::ID_UNDEFINED, "SceneNode " << name
					<< " has invalid id."  )

			msg.write(_scene_nodes.at(i).id);
		}
	}

 	if( dirtyBits & DIRTY_RELOAD_SCENE )
 	{
 		msg.write(_scene_version);
 	}
}

void
eqOgre::SceneManager::deserialize( vl::cluster::Message &msg, const uint64_t dirtyBits )
{
	if( dirtyBits & DIRTY_NODES )
	{
		// TODO this does not work dynamically, we need to track changes in the
		// vector and do mapping for those objects we want changes at runtime.
		size_t size;
		msg.read(size);
		// TODO this will leak memory
		// Also if there is already registered SceneNodes this might crash the
		// applicatation in exit.
		// The user should not remove SceneNodes to avoid this.
		_scene_nodes.resize(size);
		for( size_t i = 0; i < _scene_nodes.size(); ++i )
		{
			SceneNode *node = _scene_nodes.at(i).node;
			// FIXME this does not handle changes in the IDs it will merily
			// ignore them
			// Basicly we don't want the IDs to be changed without recreating
			// the scene node.
			msg.read(_scene_nodes.at(i).id);
			// Check for new SceneNodes
			if( !node ) //|| !node->isAttached() )
			{
				EQASSERT( _scene_nodes.at(i).id != vl::ID_UNDEFINED )

				std::cout << "SceneNode ID valid : should map the object." << std::endl;
				_mapObject( _scene_nodes.at(i) );
			}
		}
	}

	if( dirtyBits & DIRTY_RELOAD_SCENE )
	{
		msg.read(_scene_version);
	}
}

void
eqOgre::SceneManager::_mapObject( eqOgre::SceneManager::SceneNodeIDPair& node )
{
	EQASSERT( _session );

	if( !node.node )
	{
		// TODO refactor this and sync version to separate function
		node.node = SceneNode::create();
	}

	EQASSERT( node.id != vl::ID_UNDEFINED );
	EQASSERT( node.node->getID() == vl::ID_UNDEFINED );

	_session->mapObjectC( node.node, node.id );

	_new_scene_nodes.push_back( node );
}

