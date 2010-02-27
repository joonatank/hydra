#include "ogre_scene_manager.hpp"

#include "ogre_scene_node.hpp"

#include "ogre_entity.hpp"

#include <eq/net/session.h>

#include <OGRE/OgreSceneManagerEnumerator.h>
// Functors
vl::ogre::CreateNodeFunc::CreateNodeFunc( vl::graph::SceneManager *sm )
	: vl::graph::CreateNodeFunc(sm)
{}

vl::graph::SceneNode *
vl::ogre::CreateNodeFunc::operator()( uint32_t id )
{
	EQASSERT( _manager );

	vl::graph::SceneNode *node = _manager->getNode( id );
	EQASSERT( node );
	_manager->getSession()->mapObject( node, id );

	return node;
}

vl::ogre::DeleteNodeFunc::DeleteNodeFunc( vl::graph::SceneManager *sm )
	: vl::graph::DeleteNodeFunc(sm)
{}

vl::graph::SceneNode *
vl::ogre::DeleteNodeFunc::operator()( uint32_t id )
{
	EQASSERT( _manager );

	vl::graph::SceneNode *node = _manager->getNode( id );
	EQASSERT( node );
	return node;
}

vl::ogre::CreateObjectFunc::CreateObjectFunc( vl::graph::SceneManager *sm )
	: vl::graph::CreateObjectFunc(sm)
{}

vl::graph::MovableObject *
vl::ogre::CreateObjectFunc::operator()( uint32_t id )
{
	EQASSERT( _manager );

	vl::graph::MovableObject *obj = _manager->getObject( id );
	EQASSERT( obj );
	_manager->getSession()->mapObject( obj, id );
	return obj;
}

vl::ogre::DeleteObjectFunc::DeleteObjectFunc( vl::graph::SceneManager *sm )
	: vl::graph::DeleteObjectFunc(sm)
{}

vl::graph::MovableObject *
vl::ogre::DeleteObjectFunc::operator()( uint32_t id )
{
	EQASSERT( _manager );

	vl::graph::MovableObject *obj = _manager->getObject( id );
	EQASSERT( obj );
	return obj;
}

// ENDOF Functors

// SceneManager
vl::ogre::SceneManager::SceneManager( std::string const &name )
	: vl::graph::SceneManager(name), _ogre_sm(0)
{
	_ogre_sm = new Ogre::DefaultSceneManager( name );
}

// ---- protected -----
vl::graph::SceneNode *
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
	if( typeName != "Entity" )
	{ return 0; }

	// TODO check how we can create the Ogre Entity
	// We have to use Ogre::SceneManager:createEntity
	Entity *ent = new Entity( name, params );
	// Lets try to make the Ogre instance immediatly 
	ent->load( this );
	return ent;
}

void
vl::ogre::SceneManager::_createDistribContainers( void )
{
	_objectCreateFunc = new CreateObjectFunc( this );
	_objectDeleteFunc = new DeleteObjectFunc( this );

	_objects = new vl::DistributedContainer<vl::graph::MovableObject *>(
			_objectCreateFunc, _objectDeleteFunc );

	_nodeCreateFunc = new CreateNodeFunc( this );
	_nodeDeleteFunc = new DeleteNodeFunc( this );

	_nodes = new vl::DistributedContainer<vl::graph::SceneNode *>(
			_nodeCreateFunc, _nodeDeleteFunc );
}
