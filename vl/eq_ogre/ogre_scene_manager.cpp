#include "eq_ogre/ogre_scene_manager.hpp"

#include "eq_ogre/ogre_scene_node.hpp"

#include "eq_ogre/ogre_entity.hpp"

#include <eq/net/session.h>

// SceneManager
vl::ogre::SceneManager::SceneManager( Ogre::SceneManager *man, std::string const &name )
	: vl::cl::SceneManager(name), _ogre_sm( man )
{
	if( !_ogre_sm )
	{ throw vl::null_pointer("vl::ogre::SceneManager::SceneManager"); }
}

vl::ogre::SceneManager::~SceneManager( void )
{
	//	Can not call delete here, Ogre does not like it.
	//OGRE_DELETE _ogre_sm;
	// TODO delete all SceneNodes also
}

Ogre::SceneManager *
vl::ogre::SceneManager::getNative( void )
{ return _ogre_sm; }