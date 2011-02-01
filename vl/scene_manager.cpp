/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2011-01
 */

#include "scene_manager.hpp"

/// Public
vl::SceneManager::SceneManager( vl::Session *session )
	: _scene_version( 0 ), _ogre_sm(0), _session(session)
{}

vl::SceneManager::~SceneManager( void )
{
	for( size_t i = 0; i < _scene_nodes.size(); ++i )
	{ delete _scene_nodes.at(i).node; }

	_scene_nodes.clear();
}

bool
vl::SceneManager::setSceneManager( Ogre::SceneManager *man )
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
				std::cerr << "No Ogre SceneNode with name " << node->getName()
					<< " found in the SceneGraph." << std::endl;
				retval = false;
			}
		}
	}

	return retval;
}

vl::SceneNodePtr
vl::SceneManager::createSceneNode( std::string const &name )
{
	// TODO check that no two SceneNodes have the same name
	SceneNodePtr node = SceneNode::create( name );
	addSceneNode( node );
	return node;
}

void
vl::SceneManager::addSceneNode( vl::SceneNodePtr node )
{
	assert(node);
	// Check that no two nodes have the same name
	for( size_t i = 0; i < getNSceneNodes(); ++i )
	{
		SceneNodePtr ptr = getSceneNode(i);
		if( ptr == node || ptr->getName() == node->getName() )
		{
			// TODO is this the right exception?
			BOOST_THROW_EXCEPTION( vl::duplicate() );
		}
	}
	assert( _session );
	setDirty( DIRTY_NODES );

	_session->registerObjectC( node );
	assert( node->getID() != vl::ID_UNDEFINED );
	_scene_nodes.push_back( SceneNodeIDPair(node, node->getID()) );

	std::cout << "SceneNode : " << _scene_nodes.back().node->getName()
		<< " registered." << std::endl;
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
		SceneNode *node = _scene_nodes.at(i).node;
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
		SceneNode *node = _scene_nodes.at(i).node;
		if( node->getName() == name )
		{ return node; }
	}

	return 0;
}

vl::SceneNode *
vl::SceneManager::getSceneNode(size_t i)
{
	return _scene_nodes.at(i).node;
}

const vl::SceneNodePtr
vl::SceneManager::getSceneNode(size_t i) const
{
	return _scene_nodes.at(i).node;
}

void vl::SceneManager::reloadScene(void )
{
	std::cerr << "Should reload the scene now." << std::endl;
	setDirty( DIRTY_RELOAD_SCENE );
	_scene_version++;
}

void
vl::SceneManager::finaliseSync( void )
{
// 	std::cout << "vl::SceneManager::finaliseSync" << std::endl;

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
vl::SceneManager::serialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits )
{
	if( dirtyBits & DIRTY_NODES )
	{
		msg << _scene_nodes.size();
		for( size_t i = 0; i < _scene_nodes.size(); ++i )
		{
			uint64_t id = _scene_nodes.at(i).id;
			std::string name = _scene_nodes.at(i).node->getName();
			assert( id != vl::ID_UNDEFINED );

			msg << _scene_nodes.at(i).id;
		}
	}

 	if( dirtyBits & DIRTY_RELOAD_SCENE )
 	{
 		msg << _scene_version;
 	}
}

void
vl::SceneManager::deserialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits )
{
	if( dirtyBits & DIRTY_NODES )
	{
		// TODO this does not work dynamically, we need to track changes in the
		// vector and do mapping for those objects we want changes at runtime.
		size_t size;
		msg >> size;
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
			msg >> _scene_nodes.at(i).id;
			// Check for new SceneNodes
			if( !node ) //|| !node->isAttached() )
			{
				assert( _scene_nodes.at(i).id != vl::ID_UNDEFINED );

// 				std::cout << "SceneNode ID valid : should map the object." << std::endl;
				_mapObject( _scene_nodes.at(i) );
			}
		}
	}

	if( dirtyBits & DIRTY_RELOAD_SCENE )
	{
		msg >> _scene_version;
	}
}

void
vl::SceneManager::_mapObject( vl::SceneManager::SceneNodeIDPair& node )
{
	assert( _session );

	if( !node.node )
	{
		// TODO refactor this and sync version to separate function
		node.node = SceneNode::create();
	}

	assert( node.id != vl::ID_UNDEFINED );
	assert( node.node->getID() == vl::ID_UNDEFINED );

	_session->mapObjectC( node.node, node.id );

	_new_scene_nodes.push_back( node );
}

