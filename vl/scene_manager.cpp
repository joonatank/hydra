/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-01
 *	@file scene_manager.cpp
 */

#include "scene_manager.hpp"

/// Public
vl::SceneManager::SceneManager( vl::Session *session, uint64_t id )
	: _scene_version(0)
	, _ambient_light(0, 0, 0, 1)
	, _ogre_sm(0)
	, _session(session)
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
	if( _ogre_sm != man )
	{
		_ogre_sm = man;
		_ogre_sm->setAmbientLight(_ambient_light);
	}
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
	std::vector<SceneNodePtr>::iterator iter
		= std::find(_selection.begin(), _selection.end(), node);
	if( iter != _selection.end() )
	{
		_selection.erase(iter);
		node->showBoundingBox(false);
	}
}

void 
vl::SceneManager::hideSceneNodes(std::string const &pattern, bool caseInsensitive)
{
	std::string str(pattern);
	if(caseInsensitive)
	{
		vl::to_lower(str);
	}

	std::string::size_type pos = str.find('*');
	bool asteriks = false;
	if( pos != std::string::npos )
	{ asteriks = true; }

	std::string find_name = str.substr(0, pos);

	SceneNodeList::iterator iter;
	for( iter = _scene_nodes.begin(); iter != _scene_nodes.end(); ++iter )
	{
		std::string name = (*iter)->getName();
		if(caseInsensitive)
		{ vl::to_lower(name); }
		if(asteriks)
		{ name = name.substr(0, pos); }
		
		std::clog << "Test name = " << name << std::endl;
		
		if(find_name == name)
		{
			std::clog << "Found one node with name " << name << " : hiding it." << std::endl;
			(*iter)->setVisibility(false); 
		}
	}
}

bool
vl::SceneManager::isInSelection( vl::SceneNodePtr node ) const
{
	std::vector<SceneNodePtr>::const_iterator iter;
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
