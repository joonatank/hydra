
#include "frame_data.hpp"

/// Public
bool eqOgre::FrameData::findNodes(Ogre::SceneManager* man)
{
	bool retval = true;
	for( size_t i = 0; i < _scene_nodes.size(); ++i )
	{
		retval |= _scene_nodes.at(i).node->findNode(man);
	}
	return retval;
}


void eqOgre::FrameData::addSceneNode(eqOgre::SceneNode* node)
{
	setDirty( DIRTY_NODES );
	_scene_nodes.push_back( SceneNodeIDPair(node) );
}

eqOgre::SceneNode* eqOgre::FrameData::getSceneNode(const std::string& name)
{
	for( size_t i = 0; i < _scene_nodes.size(); ++i )
	{
		SceneNode *node = _scene_nodes.at(i).node;
		if( node->getName() == name )
		{ return node; }
	}

	return 0;
}

eqOgre::SceneNode* eqOgre::FrameData::getSceneNode(size_t i)
{
	return _scene_nodes.at(i).node;
}

uint32_t eqOgre::FrameData::commitAll( void )
{
	for( size_t i = 0; i < _scene_nodes.size(); ++i )
	{
		SceneNode *node = _scene_nodes.at(i).node;
		if( node->isDirty() )
		{ node->commit(); }
	}

	if( isDirty() )
	{ return commit(); }
	else
	{ return getVersion(); }
}

void
eqOgre::FrameData::syncAll( void )
{
	for( size_t i = 0; i < _scene_nodes.size(); ++i )
	{
		SceneNode *node = _scene_nodes.at(i).node;
		uint32_t id = _scene_nodes.at(i).id;
		if( !node )
		{
			std::cerr << "eqOgre::FrameData::syncAll new SceneNode" << std::endl;
			_scene_nodes.at(i).node = new SceneNode;
			node = _scene_nodes.at(i).node;
			// TODO add mapping here
			// also needs registration to be added to addSceneNode
			// TODO needs to be tested by adding objects at runtime
			getSession()->mapObject( node, _scene_nodes.at(i).id );
		}
		else if( node->getID() == EQ_ID_INVALID )
		{
			std::cerr << "eqOgre::FrameData::syncAll SceneNode ID invalid" << std::endl;
			EQASSERT( false );
		}
		else if( node->getID() != id )
		{
			std::cerr << "eqOgre::FrameData::syncAll SceneNode ID not same as distrib ID" << std::endl;
			EQASSERT( false );
		}
		else
			node->sync();
	}

	sync();
}

void
eqOgre::FrameData::registerData(eq::Config* config)
{
	for( size_t i = 0; i < _scene_nodes.size(); ++i )
	{
		SceneNode *node = _scene_nodes.at(i).node;
		// Lets make sure we don't register the objects more than once
		if( EQ_ID_INVALID != _scene_nodes.at(i).id && EQ_ID_INVALID != node->getID() )
		{
			std::cerr << "Node already registered" << std::endl;
			EQASSERT( false );
		}
		config->registerObject( node);
		// The object has to be correctly registered
		if( EQ_ID_INVALID == node->getID() )
		{
			std::cerr << "Node was not registered" << std::endl;
			EQASSERT( false );
		}
		_scene_nodes.at(i).id = node->getID();
	}

	// Register FrameData
	if( EQ_ID_INVALID != getID() )
	{
		std::cerr << "FrameData already registered" << std::endl;
		EQASSERT( false );
	}
	config->registerObject( this );
	if( EQ_ID_INVALID == getID() )
	{
		std::cerr << "FrameData was not registered" << std::endl;
		EQASSERT( false );
	}
}

void
eqOgre::FrameData::deregisterData(eq::Config* config)
{
	for( size_t i = 0; i < _scene_nodes.size(); ++i )
	{
		SceneNode *node = _scene_nodes.at(i).node;
		if( EQ_ID_INVALID != node->getID() )
		{ config->deregisterObject( node ); }
		_scene_nodes.at(i).id = EQ_ID_INVALID;
	}

	if( EQ_ID_INVALID != getID() )
	{ config->deregisterObject( this ); }
}

void
eqOgre::FrameData::mapData(eq::Config* config, uint32_t id)
{
	// We need to map this object first so that we have valid _scene_nodes vector
	if( EQ_ID_INVALID == id )
	{
		std::cerr << "Trying to map to invalid ID" << std::endl;
		EQASSERT( false );
	}
	config->mapObject(this, id);

	for( size_t i = 0; i < _scene_nodes.size(); ++i )
	{
		SceneNode *node = _scene_nodes.at(i).node;
		if( !node )
		{
			// TODO refactor this and sync version to separate function
			_scene_nodes.at(i).node = new SceneNode;
			node = _scene_nodes.at(i).node;
		}
		
		if( EQ_ID_INVALID == _scene_nodes.at(i).id )
		{
			std::cerr << "Trying to map Ogre to invalid ID" << std::endl;
			EQASSERT( false );
		}
		config->mapObject( node, _scene_nodes.at(i).id );
	}
}

void
eqOgre::FrameData::unmapData(eq::Config* config)
{
	for( size_t i = 0; i < _scene_nodes.size(); ++i )
	{
		SceneNode *node = _scene_nodes.at(i).node;
		config->unmapObject( node );
	}

	config->unmapObject( this );
}


// ------ Protected --------
void 
eqOgre::FrameData::serialize( eq::net::DataOStream &os, const uint64_t dirtyBits )
{
	eq::fabric::Serializable::serialize( os, dirtyBits );

	if( dirtyBits & DIRY_HEAD )
	{
		os << _head_pos << _head_orient;
	}


	if( dirtyBits & DIRTY_NODES )
	{
		os << _scene_nodes.size();
		for( size_t i = 0; i < _scene_nodes.size(); ++i )
		{
			os << _scene_nodes.at(i).id;
		}
	}

	if( dirtyBits & DIRTY_RELOAD_SCENE )
	{
		os << _scene_version;
	}

	if( dirtyBits & DIRTY_RESET_SCENE )
	{
	}
}

void 
eqOgre::FrameData::deserialize( eq::net::DataIStream &is, const uint64_t dirtyBits )
{
	eq::fabric::Serializable::deserialize( is, dirtyBits );

	if( dirtyBits & DIRY_HEAD )
	{
		is >> _head_pos >> _head_orient;
	}

	if( dirtyBits & DIRTY_NODES )
	{
		// TODO this does not work dynamically, we need to track changes in the
		// vector and do mapping for those objects we want changes at runtime.
		size_t size;
		is >> size;
		// TODO this will leak memory
		_scene_nodes.resize(size);
		for( size_t i = 0; i < _scene_nodes.size(); ++i )
		{
			is >> _scene_nodes.at(i).id;
			if( _scene_nodes.at(i).id == EQ_ID_INVALID )
			{
				std::cerr << "SceneNode ID invalid when deserializing!" << std::endl;
				EQASSERT( false );
			}
		}
	}

	if( dirtyBits & DIRTY_RELOAD_SCENE )
	{
		is >> _scene_version;
	}

	if( dirtyBits & DIRTY_RESET_SCENE )
	{
	}
}
