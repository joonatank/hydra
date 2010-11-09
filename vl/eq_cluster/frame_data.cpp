
#include "frame_data.hpp"

/// Public
eqOgre::FrameData::FrameData( void )
	: Serializable(), _scene_version( 0 ), _config(0), _ogre_sm(0)
{}

eqOgre::FrameData::~FrameData(void )
{
	for( size_t i = 0; i < _scene_nodes.size(); ++i )
	{
		delete _scene_nodes.at(i).node;
	}
	_scene_nodes.clear();
}

bool eqOgre::FrameData::setSceneManager(Ogre::SceneManager* man)
{
	if( !man )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	_ogre_sm = man;

	bool retval = true;
	for( size_t i = 0; i < _scene_nodes.size(); ++i )
	{
		SceneNode *node = _scene_nodes.at(i).node;
		EQASSERT( node );
		if( !node->findNode(man) )
		{
			std::cerr << "No Ogre SceneNode with name " << node->getName()
				<< " found in the SceneGraph." << std::endl;
			retval = false;
		}
	}

	return retval;
}

void eqOgre::FrameData::addSceneNode(eqOgre::SceneNode* node)
{
	setDirty( DIRTY_NODES );
	_scene_nodes.push_back( SceneNodeIDPair(node) );

	if( _scene_nodes.back().id == EQ_ID_INVALID )
	{
		if( _config )
		{ _registerObject( _scene_nodes.back() ); }
		std::cerr << "Object Registered" << std::endl;
	}
}

eqOgre::SceneNode *
eqOgre::FrameData::getSceneNode(const std::string& name)
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
eqOgre::FrameData::getSceneNode(size_t i)
{
	return _scene_nodes.at(i).node;
}

uint32_t eqOgre::FrameData::commitAll( void )
{
	for( size_t i = 0; i < _scene_nodes.size(); ++i )
	{
		SceneNode *node = _scene_nodes.at(i).node;
		node->commit();
	}

//	std::cout << "FrameData dirty mask = " << std::hex << getDirty()
//		<< std::dec << std::endl;
	return commit();
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
			_scene_nodes.at(i).node = SceneNode::create();
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
	std::cerr << "eqOgre::FrameData::registerData" << std::endl;
	EQASSERT( config );

	// Config is valid, lets save it so we can register child objects
	_config = config;
	std::cerr << "Registering " << _scene_nodes.size() << " SceneNodes." << std::endl;
	for( size_t i = 0; i < _scene_nodes.size(); ++i )
	{
		_registerObject( _scene_nodes.at(i) );
	}

	// Register FrameData
	// NOTE This will Serialize the FrameData, so the childs should be already
	// registered!
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
		std::cerr << "Trying to map FrameData invalid ID" << std::endl;
		EQASSERT( false );
	}
	config->mapObject(this, id);
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
// NOTE No registering can be done in the serialize method, it's called from
// different thread.
// FIXME  serialize is called from getInstanceData (DIRTY_ALL) when new version
// has been commited, why?
void 
eqOgre::FrameData::serialize( eq::net::DataOStream &os, const uint64_t dirtyBits )
{
	eq::fabric::Serializable::serialize( os, dirtyBits );

//	std::cerr << "eqOgre::FrameData::serialize mask = " << std::hex << dirtyBits
//		<< std::dec << std::endl;

	if( dirtyBits & DIRTY_NODES )
	{
//		std::cerr << "this = " << this << " : Serializing " << _scene_nodes.size() << " SceneNodes." << std::endl;
		os << _scene_nodes.size();
		for( size_t i = 0; i < _scene_nodes.size(); ++i )
		{

			if( _scene_nodes.at(i).id == EQ_ID_INVALID )
			{
				std::cerr << "SceneNode " << _scene_nodes.at(i).node->getName()
					<< " has invalid id." << std::endl;
				EQASSERT( false );
			}
			os << _scene_nodes.at(i).id;
		}
	}

	if( dirtyBits & DIRTY_RELOAD_SCENE )
	{
//		std::cerr << "this = " << this << " : Serializing Reload Scene." << std::endl;
		os << _scene_version;
	}

	if( dirtyBits & DIRTY_RESET_SCENE )
	{
//		std::cerr << "this = " << this << " : Serializing Reset Scene." << std::endl;
	}
}

void 
eqOgre::FrameData::deserialize( eq::net::DataIStream &is, const uint64_t dirtyBits )
{
	eq::fabric::Serializable::deserialize( is, dirtyBits );

	if( dirtyBits & DIRTY_NODES )
	{
		// TODO this does not work dynamically, we need to track changes in the
		// vector and do mapping for those objects we want changes at runtime.
		size_t size;
		is >> size;
		// TODO this will leak memory
		_scene_nodes.resize(size);
//		std::cerr << "Deserialize : " << size << " SceneNodes." << std::endl;
		for( size_t i = 0; i < _scene_nodes.size(); ++i )
		{
			SceneNode *node = _scene_nodes.at(i).node;
			// FIXME this does not handle changes in the IDs it will merily ignore
			// them
			is >> _scene_nodes.at(i).id;
			// Check for new SceneNodes
			if( !node || (node->getID() == EQ_ID_INVALID) )
			{
				if( _scene_nodes.at(i).id == EQ_ID_INVALID )
				{
					std::cerr << "SceneNode ID invalid when deserializing!" << std::endl;
					//EQASSERT( false );
				}
				else
				{
					std::cerr << "SceneNode ID valid : mapping object." << std::endl;
					_mapObject( _scene_nodes.at(i) );
				}
			}
		}
	}

	if( dirtyBits & DIRTY_RELOAD_SCENE )
	{
//		std::cerr << "Deserialize : reload scene "<< std::endl;
		is >> _scene_version;
	}

	if( dirtyBits & DIRTY_RESET_SCENE )
	{
//		std::cerr << "Deserialize : reset scene "<< std::endl;
	}
}

void eqOgre::FrameData::_mapObject(eqOgre::FrameData::SceneNodeIDPair& node)
{
	std::cerr << "eqOgre::FrameData::_mapObject" << std::endl;

	if( !getSession() )
	{
		std::cerr << "No session." << std::endl;
		EQASSERT( false );
	}

	if( !node.node )
	{
		// TODO refactor this and sync version to separate function
		node.node = SceneNode::create();
	}
	if( EQ_ID_INVALID != node.node->getID() )
	{
		std::cerr << "Node ID is valid! Mapping will fail." << std::endl;
	}
	
	if( EQ_ID_INVALID == node.id )
	{
		std::cerr << "Trying to map object to invalid ID" << std::endl;
		EQASSERT( false );
	}

	getSession()->mapObject( node.node, node.id );

	// Find ogre nodes
	if( _ogre_sm )
	{
		if( !node.node->findNode( _ogre_sm ) )
		{
			std::cerr << "No Ogre SceneNode with name " << node.node->getName()
				<< " found in the SceneGraph." << std::endl;
		}
	}
}

void eqOgre::FrameData::_registerObject(eqOgre::FrameData::SceneNodeIDPair& node)
{
	std::cerr << "Registering Object : " << node.node->getName() << std::endl;

	if( !_config )
	{
		std::cerr << "No Config when registering object." << std::endl;
		EQASSERT( false );
	}

	// Lets make sure we don't register the objects more than once
	if( EQ_ID_INVALID != node.id && EQ_ID_INVALID != node.node->getID() )
	{
		std::cerr << "Node already registered" << std::endl;
		EQASSERT( false );
	}

	std::cerr << "config->registerObject" << std::endl;
	// FIXME this crashes
	_config->registerObject( node.node);
	std::cerr << "config->registerObject done" << std::endl;
	// The object has to be correctly registered
	if( EQ_ID_INVALID == node.node->getID() )
	{
		std::cerr << "Node was not registered" << std::endl;
		EQASSERT( false );
	}

	node.id = node.node->getID();
}
