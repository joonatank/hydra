/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-01
 *	@file scene_manager.cpp
 */

#include "scene_manager.hpp"

#include "scene_node.hpp"
#include "entity.hpp"

/// Public
vl::SceneManager::SceneManager( vl::Session *session, uint64_t id )
	: _root(0)
	, _scene_version(0)
	, _ambient_light(0, 0, 0, 1)
	, _session(session)
	, _ogre_sm(0)
{
	assert( _session );
	_session->registerObject( this, OBJ_SCENE_MANAGER, id );
	_root = _createSceneNode("Root", vl::ID_UNDEFINED);
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
	if( _ogre_sm != man )
	{
		_ogre_sm = man;
		_ogre_sm->setAmbientLight(_ambient_light);
	}
}

vl::SceneNodePtr
vl::SceneManager::createSceneNode(std::string const &name)
{
	vl::SceneNodePtr node = _createSceneNode(name, vl::ID_UNDEFINED);
	assert(_root);
	_root->addChild(node);
	return node;
}

vl::SceneNodePtr
vl::SceneManager::createFreeSceneNode(std::string const &name)
{
	return _createSceneNode(name, vl::ID_UNDEFINED);
}

vl::SceneNodePtr
vl::SceneManager::_createSceneNode(uint64_t id)
{
	assert(vl::ID_UNDEFINED != id);
	return _createSceneNode(std::string(), id);
}

bool
vl::SceneManager::hasSceneNode(const std::string& name) const
{
	return( getSceneNode(name) );
}

vl::SceneNodePtr
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

vl::SceneNodePtr
vl::SceneManager::getSceneNodeID(uint64_t id) const
{
	for( size_t i = 0; i < _scene_nodes.size(); ++i )
	{
		SceneNode *node = _scene_nodes.at(i);
		if( node->getID() == id )
		{ return node; }
	}

	return 0;
}

/// --------------------- SceneManager Entity --------------------------------
vl::EntityPtr
vl::SceneManager::createEntity(std::string const &name, vl::PREFAB type)
{
	/// Disallow empty names for now, we need to generate one otherwise
	if(name.empty())
	{ BOOST_THROW_EXCEPTION( vl::empty_param() ); }
	if(hasEntity(name))
	{ BOOST_THROW_EXCEPTION( vl::duplicate() << vl::name(name) ); }

	EntityPtr ent = new Entity(name, type, this);
	
	_session->registerObject( ent, OBJ_ENTITY, vl::ID_UNDEFINED );
	assert( ent->getID() != vl::ID_UNDEFINED );
	_entities.push_back(ent);

	return ent;
}

vl::EntityPtr
vl::SceneManager::createEntity(std::string const &name, std::string const &mesh_name)
{
	/// Disallow empty names for now, we need to generate one otherwise
	if(name.empty())
	{ BOOST_THROW_EXCEPTION( vl::empty_param() ); }
	if(hasEntity(name))
	{ BOOST_THROW_EXCEPTION( vl::duplicate() << vl::name(name) ); }

	EntityPtr ent = new Entity(name, mesh_name, this);
	
	_session->registerObject(ent, OBJ_ENTITY, vl::ID_UNDEFINED);
	assert( ent->getID() != vl::ID_UNDEFINED );
	_entities.push_back(ent);

	return ent;
}

vl::EntityPtr
vl::SceneManager::_createEntity(uint64_t id)
{
	if( vl::ID_UNDEFINED == id )
	{ BOOST_THROW_EXCEPTION( vl::invalid_id() ); }

	EntityPtr ent = new Entity("", "", this);
	
	_session->registerObject(ent, OBJ_ENTITY, id);
	assert( ent->getID() != vl::ID_UNDEFINED );
	_entities.push_back(ent);

	return ent;
}

bool 
vl::SceneManager::hasEntity( std::string const &name ) const
{
	return( getEntity(name) );
}

vl::EntityPtr 
vl::SceneManager::getEntity( std::string const &name ) const
{
	for( EntityList::const_iterator iter = _entities.begin(); iter != _entities.end(); ++iter )
	{
		if( (*iter)->getName() == name )
		{ return *iter; }
	}

	return 0;
}

vl::EntityPtr
vl::SceneManager::getEntityID(uint64_t id) const
{
	for( size_t i = 0; i < _entities.size(); ++i )
	{
		EntityPtr ent = _entities.at(i);
		if( ent->getID() == id )
		{ return ent; }
	}

	return 0;
}

void
vl::SceneManager::reloadScene( void )
{
	std::cerr << "Should reload the scene now." << std::endl;
	setDirty( DIRTY_RELOAD_SCENE );
	_scene_version++;
}

void
vl::SceneManager::addToSelection( vl::SceneNodePtr node )
{
	if( !isInSelection(node) )
	{
		_selection.push_back(node);
		node->showBoundingBox(true);
	}
}

void
vl::SceneManager::removeFromSelection( vl::SceneNodePtr node )
{
	SceneNodeList::iterator iter
		= std::find(_selection.begin(), _selection.end(), node);
	if( iter != _selection.end() )
	{
		_selection.erase(iter);
		node->showBoundingBox(false);
	}
}

bool
vl::SceneManager::isInSelection( vl::SceneNodePtr node ) const
{
	SceneNodeList::const_iterator iter;
	for( iter = _selection.begin(); iter != _selection.end(); ++iter )
	{
		if( *iter == node )
		{ return true; }
	}

	return false;
}


/// -------------------------------Protected -----------------------------------
void
vl::SceneManager::serialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits )
{
	if( dirtyBits & DIRTY_RELOAD_SCENE )
	{
		msg << _scene_version;
	}

	
	if( dirtyBits & DIRTY_AMBIENT_LIGHT )
	{
		msg << _ambient_light.r << _ambient_light.g << _ambient_light.b << _ambient_light.a;
	}
}

void
vl::SceneManager::deserialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits )
{
	if( dirtyBits & DIRTY_RELOAD_SCENE )
	{
		msg >> _scene_version;
	}
	
	if( dirtyBits & DIRTY_AMBIENT_LIGHT )
	{
		msg >> _ambient_light.r >> _ambient_light.g >> _ambient_light.b >> _ambient_light.a;
		if( _ogre_sm )
		{
			_ogre_sm->setAmbientLight(_ambient_light);
		}
	}
}

/// ------------------------------- Private ----------------------------------
vl::SceneNodePtr 
vl::SceneManager::_createSceneNode(std::string const &name, uint64_t id)
{
	assert( !name.empty() || vl::ID_UNDEFINED != id );

	// Check that no two nodes have the same name
	if( !name.empty() && hasSceneNode(name) )
	{
		// TODO is this the right exception?
		BOOST_THROW_EXCEPTION( vl::duplicate() );
	}
	assert( _session );

	SceneNodePtr node = new SceneNode( name, this );

	_session->registerObject( node, OBJ_SCENE_NODE, id );
	assert( node->getID() != vl::ID_UNDEFINED );
	_scene_nodes.push_back( node );

	return node;
}
