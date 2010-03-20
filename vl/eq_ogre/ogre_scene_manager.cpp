#include "eq_ogre/ogre_scene_manager.hpp"

#include "eq_ogre/ogre_scene_node.hpp"

#include "eq_ogre/ogre_entity.hpp"

#include <eq/net/session.h>

#include <OGRE/OgreSceneManagerEnumerator.h>

// SceneManager
vl::ogre::SceneManager::SceneManager( std::string const &name )
	: vl::cl::SceneManager(name), _ogre_sm(0)
{
	_ogre_sm = new Ogre::DefaultSceneManager( name );
}

vl::ogre::SceneManager::~SceneManager( void )
{
	//	Can not call delete here, Ogre does not like it.
//	delete _ogre_sm;
	// TODO delete all SceneNodes also
}

vl::graph::Camera *
vl::ogre::SceneManager::createCamera( std::string const &name )
{
	Ogre::Camera *ogre_cam = _ogre_sm->createCamera(name);
	return new Camera( ogre_cam );
}

// ---- protected -----
vl::cl::SceneNode *
vl::ogre::SceneManager::_createSceneNodeImpl( std::string const &name )
{
	return new vl::ogre::SceneNode( this, name );
}

vl::graph::MovableObject *
vl::ogre::SceneManager::_createMovableObjectImpl(
		std::string const &typeName,
		std::string const &name,
		vl::NamedValuePairList const &params )
{
	// For now we only use entities
	// TODO this should throw
	if( typeName != "Entity" )
	{ return 0; }

	// TODO check how we can create the Ogre Entity
	// We have to use Ogre::SceneManager:createEntity
	Entity *ent = new Entity( name, params );

	// Lets try to make the Ogre instance immediatly 
	//ent->load( this );
	return ent;
}

