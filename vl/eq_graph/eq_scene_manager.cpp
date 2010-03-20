#include "eq_scene_manager.hpp"

#include <eq/net/session.h>

// SceneManager
vl::cl::SceneManager::SceneManager( std::string const &name )
	: _root(0)
{
	if( name.empty() )
	{ throw vl::empty_param("vl::cl::SceneManager::SceneManager"); }

	eq::Object::setName( name );
}

vl::graph::SceneNode *
vl::cl::SceneManager::createNode( std::string const &name )
{
	vl::cl::SceneNode *node = _createSceneNodeImpl( name );

	// TODO add to nodes, we should also share it here so there
	// will be ID generated for this node
	// Seems not to need to be registered.
	// Um the ID is invalid so we need to register the scene_manager first
	// or register the objects later.
	//getSession()->registerObject( node );

	// This needs the ID to be generated already so we need to
	// register the node first.
	//_nodes->push( node );
	//_nodes->commit();
	return node;
}

vl::graph::Entity *
vl::cl::SceneManager::createEntity(
		std::string const &name, std::string const &meshName )
{
	vl::NamedValuePairList params;
	params["mesh"] = meshName;
	vl::graph::MovableObject *obj = _createMovableObjectImpl(
			"Entity", name, params );

	return (Entity *)obj;
}

// Find function needs scene graph traversal to be implemented
vl::graph::SceneNode *
vl::cl::SceneManager::getNode( std::string const &name )
{
	return 0;
}

// Find function needs scene graph traversal to be implemented
vl::graph::SceneNode *
vl::cl::SceneManager::getNode( uint32_t id )
{
	/*
	for( size_t i = 0; i < _nodes->size(); i++ )
	{
		if( _nodes->at(i)->getID() == id )
		{ return _nodes->at(i); }
	}
	*/

	return 0;
}

// Find function needs scene graph traversal to be implemented
vl::graph::MovableObject *
vl::cl::SceneManager::getObject( uint32_t id )
{
	/*
	for( size_t i = 0; i < _objects->size(); i++ )
	{
		if( _objects->at(i)->getID() == id )
		{ return _objects->at(i); }
	}
	*/

	return 0;
}

// equalizer overrides
void
vl::cl::SceneManager::serialize( eq::net::DataOStream& os,
		const uint64_t dirtyBits )
{
	if( DIRTY_ROOT & dirtyBits )
	{
	}
}

void
vl::cl::SceneManager::deserialize( eq::net::DataIStream& is,
						  const uint64_t dirtyBits )
{
	if( DIRTY_ROOT & dirtyBits )
	{
	}
}

vl::cl::SceneNode *
vl::cl::SceneManager::_createSceneNodeImpl( std::string const &name )
{
	return new vl::cl::SceneNode( this, name );
}

vl::graph::MovableObject *
vl::cl::SceneManager::_createMovableObjectImpl(
		std::string const &typeName, std::string const &name,
		vl::NamedValuePairList const &params )
{
	// For now we only use entities
	if( typeName != "Entity" )
	{ return 0; }

	return new vl::cl::Entity( name, params );
}

