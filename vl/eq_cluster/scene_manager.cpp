
#include "scene_manager.hpp"

/// Public
eqOgre::SceneManager::SceneManager( void )
	: Serializable(), _scene_version( 0 ), _ogre_sm(0), _config(0)
{}

eqOgre::SceneManager::~SceneManager( void )
{
	for( size_t i = 0; i < _scene_nodes.size(); ++i )
	{
		delete _scene_nodes.at(i).node;
	}
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
	setDirty( DIRTY_NODES );
	_scene_nodes.push_back( SceneNodeIDPair(node) );

	if( !_scene_nodes.back().node->isAttached() )
	{
		if( _config )
		{
			_registerObject( _scene_nodes.back() );
			EQINFO << "SceneNode : " << _scene_nodes.back().node->getName()
				<< " registered." << std::endl;
		}
		else
		{ EQERROR << "Not in a session : not registering new object." << std::endl; }
	}
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

eq::uint128_t
eqOgre::SceneManager::commitAll( void )
{
	for( size_t i = 0; i < _scene_nodes.size(); ++i )
	{
		SceneNode *node = _scene_nodes.at(i).node;
		node->commit();
	}
	return commit();
}

void
eqOgre::SceneManager::syncAll( void )
{
	EQASSERT( _config )

	for( size_t i = 0; i < _scene_nodes.size(); ++i )
	{
		SceneNode *node = _scene_nodes.at(i).node;
		eq::base::UUID const &id = _scene_nodes.at(i).id;
		// If we have new scene nodes in the master stack we create matching
		// objects for the slave here and map it to the master copy.
		if( !node )
		{
			EQINFO << "eqOgre::FrameData::syncAll new SceneNode" << std::endl;
			_scene_nodes.at(i).node = SceneNode::create();
			node = _scene_nodes.at(i).node;
			_config->mapObject( node, id );
		}

		EQASSERTINFO( node->isAttached(), "SceneNode is not attached." );

		EQASSERTINFO( node->getID() == id, "SceneNode ID not same as distrib ID." )

		node->sync();
	}

	sync();
}

bool
eqOgre::SceneManager::registerData( eq::Config *session )
{
	EQINFO << "eqOgre::FrameData::registerData" << std::endl;
	EQASSERT( session );
	// Disallow registering multiple times
	EQASSERT( !_config );
	_config = session;

	bool retval = true;
	// SceneNodes need to be registered first for them to have correct ids
	// when FrameData is registered
	EQINFO << "Registering " << _scene_nodes.size() << " SceneNodes." << std::endl;
	for( size_t i = 0; i < _scene_nodes.size(); ++i )
	{
		retval = retval && _registerObject( _scene_nodes.at(i) );
	}

	// Register FrameData
	if( !this->isAttached() )
	{
		retval = retval && _config->registerObject( this );
	}

	return retval;
}

void
eqOgre::SceneManager::deregisterData( void )
{
	EQASSERT( _config );

	for( size_t i = 0; i < _scene_nodes.size(); ++i )
	{
		SceneNode *node = _scene_nodes.at(i).node;
		if( node && node->isAttached() )
		{ _config->deregisterObject( node ); }

		_scene_nodes.at(i).id = eq::base::UUID::ZERO;
		// TODO this might need to destroy the SceneNode
	}

	_config->deregisterObject( this );
}

bool
eqOgre::SceneManager::mapData( eq::Config *session, eq::base::UUID const &id )
{
	// We need to map this object first so that we have valid _scene_nodes vector
	EQASSERTINFO( eq::base::UUID::ZERO != id, "Trying to map FrameData invalid ID" )
	EQASSERTINFO( session, "mapping is only possible if we have a config" )

	_config = session;

	EQASSERTINFO( !this->isAttached(), "FrameData already mapped" )
	return _config->mapObject(this, id);
}

void
eqOgre::SceneManager::unmapData( void )
{
	EQASSERT( _config );

	for( size_t i = 0; i < _scene_nodes.size(); ++i )
	{
		SceneNode *node = _scene_nodes.at(i).node;
		if( node && node->isAttached() )
		{ _config->unmapObject( node ); }
	}

	_config->unmapObject( this );
}


// ------ Protected --------
// NOTE No registering can be done in the serialize method, it's called from
// different thread.
// FIXME  serialize is called from getInstanceData (DIRTY_ALL) when new version
// has been commited, why?
void
eqOgre::SceneManager::serialize(  co::DataOStream &os, const uint64_t dirtyBits )
{
	eq::fabric::Serializable::serialize( os, dirtyBits );

	if( dirtyBits & DIRTY_NODES )
	{
		os << _scene_nodes.size();
		for( size_t i = 0; i < _scene_nodes.size(); ++i )
		{
			eq::base::UUID const &id = _scene_nodes.at(i).id;
			std::string name = _scene_nodes.at(i).node->getName();
			EQASSERTINFO( id != eq::base::UUID::ZERO, "SceneNode " << name
					<< " has invalid id."  )

			os << _scene_nodes.at(i).id;
		}
	}

	if( dirtyBits & DIRTY_RELOAD_SCENE )
	{
		os << _scene_version;
	}
}

void
eqOgre::SceneManager::deserialize( co::DataIStream &is, const uint64_t dirtyBits )
{
	eq::fabric::Serializable::deserialize( is, dirtyBits );

	if( dirtyBits & DIRTY_NODES )
	{
		// TODO this does not work dynamically, we need to track changes in the
		// vector and do mapping for those objects we want changes at runtime.
		size_t size;
		is >> size;
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
			is >> _scene_nodes.at(i).id;
			// Check for new SceneNodes
			if( !node || !node->isAttached() )
			{
				if( _scene_nodes.at(i).id == eq::base::UUID::ZERO )
				{
					EQERROR << "SceneNode ID invalid when deserializing!" << std::endl;
				}
				else
				{
					EQERROR << "SceneNode ID valid : should map the object." << std::endl;
					_mapObject( _scene_nodes.at(i) );
				}
			}
		}
	}

	if( dirtyBits & DIRTY_RELOAD_SCENE )
	{
		is >> _scene_version;
	}
}

void
eqOgre::SceneManager::_mapObject( eqOgre::SceneManager::SceneNodeIDPair& node )
{
	EQINFO  << "eqOgre::FrameData::_mapObject" << std::endl;

	EQASSERT( _config );

	if( !node.node )
	{
		// TODO refactor this and sync version to separate function
		node.node = SceneNode::create();
	}

	EQASSERTINFO( !node.node->isAttached(), "Node is already attached!" )

	EQASSERTINFO( eq::base::UUID::ZERO != node.id, "Trying to map object to invalid ID" );

	_config->mapObject( node.node, node.id );

	// Find ogre nodes
	if( _ogre_sm )
	{
		if( !node.node->findNode( _ogre_sm ) )
		{
			EQINFO << "No Ogre SceneNode with name " << node.node->getName()
				<< " found in the SceneGraph." << std::endl;
		}
	}
}

bool
eqOgre::SceneManager::_registerObject( eqOgre::SceneManager::SceneNodeIDPair& node )
{
	EQINFO << "Registering Object : " << node.node->getName() << std::endl;

	EQASSERTINFO( _config, "No Config when registering object."  );

	EQASSERTINFO( node.node, "No Node object to register. What you trying to pull." );
	// Lets make sure we don't register the objects more than once
	EQASSERTINFO( !node.node->isAttached(), "Node already registered" )

	bool retval = _config->registerObject( node.node);

	// The object has to be correctly registered
	EQASSERTINFO( node.node->isAttached(), "Node was not registered" );

	node.id = node.node->getID();

	return retval;
}
