#include "eq_ogre/ogre_scene_manager.hpp"

#include "eq_ogre/ogre_scene_node.hpp"

#include "eq_ogre/ogre_entity.hpp"

#include <eq/net/session.h>

#include <OGRE/OgreSceneManagerEnumerator.h>

// Functors
/*
vl::ogre::CreateNodeFunc::CreateNodeFunc( vl::cl::SceneManager *sm )
	: vl::cl::CreateNodeFunc(sm)
{}

vl::cl::SceneNode *
vl::ogre::CreateNodeFunc::operator()( uint32_t id )
{
	EQASSERT( _manager );

	vl::cl::SceneNode *node = (vl::cl::SceneNode *)_manager->getNode( id );
	EQASSERT( node );
	_manager->getSession()->mapObject( node, id );

	return node;
}

vl::ogre::DeleteNodeFunc::DeleteNodeFunc( vl::cl::SceneManager *sm )
	: vl::cl::DeleteNodeFunc(sm)
{}

vl::cl::SceneNode *
vl::ogre::DeleteNodeFunc::operator()( uint32_t id )
{
	EQASSERT( _manager );

	vl::cl::SceneNode *node = (vl::cl::SceneNode *)_manager->getNode( id );
	EQASSERT( node );
	return node;
}

vl::ogre::CreateObjectFunc::CreateObjectFunc( vl::cl::SceneManager *sm )
	: vl::cl::CreateObjectFunc(sm)
{}

vl::cl::MovableObject *
vl::ogre::CreateObjectFunc::operator()( uint32_t id )
{
	EQASSERT( _manager );

	vl::cl::MovableObject *obj
		= (vl::cl::MovableObject *)_manager->getObject( id );
	EQASSERT( obj );
	_manager->getSession()->mapObject( obj, id );
	return obj;
}

vl::ogre::DeleteObjectFunc::DeleteObjectFunc( vl::cl::SceneManager *sm )
	: vl::cl::DeleteObjectFunc(sm)
{}

vl::cl::MovableObject *
vl::ogre::DeleteObjectFunc::operator()( uint32_t id )
{
	EQASSERT( _manager );

	vl::cl::MovableObject *obj
		= (vl::cl::MovableObject *)_manager->getObject( id );
	EQASSERT( obj );
	return obj;
}

*/
// ENDOF Functors

// SceneManager
vl::ogre::SceneManager::SceneManager( std::string const &name )
	: vl::cl::SceneManager(name), _ogre_sm(0)
{
	_ogre_sm = new Ogre::DefaultSceneManager( name );
}

// ---- protected -----
vl::cl::SceneNode *
vl::ogre::SceneManager::_createSceneNodeImpl( std::string const &name )
{
	return new vl::ogre::SceneNode( this, name );
}

vl::cl::MovableObject *
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
	ent->load( this );
	return ent;
}

/*
void
vl::ogre::SceneManager::_createDistribContainers( void )
{
	_objectCreateFunc = new CreateObjectFunc( this );
	_objectDeleteFunc = new DeleteObjectFunc( this );

	_objects = new vl::DistributedContainer<vl::cl::MovableObject *>(
			_objectCreateFunc, _objectDeleteFunc );

	_nodeCreateFunc = new CreateNodeFunc( this );
	_nodeDeleteFunc = new DeleteNodeFunc( this );

	_nodes = new vl::DistributedContainer<vl::cl::SceneNode *>(
			_nodeCreateFunc, _nodeDeleteFunc );
}
*/
