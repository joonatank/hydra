#include "eq_scene_manager.hpp"

#include "interface/movable_object.hpp"
#include "interface/entity.hpp"
#include "eq_scene_node.hpp"

#include <eq/net/session.h>

// SceneManager
vl::cl::SceneManager::SceneManager( std::string const &name )
	: _root()
{
	if( name.empty() )
	{ throw vl::empty_param("vl::cl::SceneManager::SceneManager"); }

	eq::Object::setName( name );
}

vl::cl::SceneManager::~SceneManager( void )
{
}

void
vl::cl::SceneManager::destroyGraph( void )
{
	// Delete the whole graph
	_root.reset();
}

vl::graph::SceneNodeRefPtr
vl::cl::SceneManager::getRootNode( void )
{
	if( !_root )
	{ _root = createNode( "Root" ); }
	return _root;
}

vl::graph::SceneNodeRefPtr
vl::cl::SceneManager::createNode( std::string const &name )
{
	const char *where = "vl::cl::SceneManager::createNode";

	// TODO replace with the factory
	//	= _createSceneNodeImpl( name );
//	if( !_scene_node_factory )
//	{ _scene_node_factory.reset(new DefaultSceneNodeFactory); }

	if( !_scene_node_factory )
	{ throw vl::exception( "No SceneNodeFactory", where ); }

	vl::graph::SceneNodeRefPtr node =
		_scene_node_factory->create( shared_from_this(), name );

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

vl::graph::EntityRefPtr
vl::cl::SceneManager::createEntity(
		std::string const &name, std::string const &meshName )
{
	vl::NamedValuePairList params;
	params["mesh"] = meshName;
	vl::graph::MovableObjectRefPtr obj = createMovableObject(
			"Entity", name, params );

	return boost::static_pointer_cast<vl::graph::Entity>( obj );
}

vl::graph::MovableObjectRefPtr
vl::cl::SceneManager::createMovableObject(
		std::string const &typeName, std::string const &name,
		vl::NamedValuePairList const &params )
{
	// For now we only use entities
	std::map<std::string, vl::graph::MovableObjectFactoryPtr>::iterator iter;
	iter = _movable_factories.find( typeName );
	if( iter != _movable_factories.end() )
	{ return iter->second->create( name, params ); }

	return vl::graph::MovableObjectRefPtr();
}

// TODO Find function needs scene graph traversal to be implemented
vl::graph::SceneNodeRefPtr
vl::cl::SceneManager::getNode( std::string const &name )
{
	return vl::graph::SceneNodeRefPtr();
}

// TODO Find function needs scene graph traversal to be implemented
vl::graph::SceneNodeRefPtr
vl::cl::SceneManager::getNode( uint32_t id )
{
	return vl::graph::SceneNodeRefPtr();
}

// TODO Find function needs scene graph traversal to be implemented
vl::graph::MovableObjectRefPtr
vl::cl::SceneManager::getObject( uint32_t id )
{
	return vl::graph::MovableObjectRefPtr();
}

// TODO implement
void
vl::cl::SceneManager::pushChildAddedStack( uint32_t id,
		vl::graph::ChildAddedFunctor const &handle )
{

}

// TODO implement
void
vl::cl::SceneManager::pushChildRemovedStack( vl::graph::SceneNodeRefPtr child )
{

}

void
vl::cl::SceneManager::setSceneNodeFactory(
		vl::graph::SceneNodeFactoryPtr factory )
{
	const char *where = "vl::cl::SceneManager::setSceneNodeFactory";
	if( !factory )
	{ throw vl::null_pointer( where ); }

	if( _root )
	{
		const char *err
			="Trying to change Scene Node Factory when Scene Graph Exists";
		throw vl::exception( err, where );
	}
	_scene_node_factory = factory;
}

void
vl::cl::SceneManager::addMovableObjectFactory(
		vl::graph::MovableObjectFactoryPtr factory, bool overwrite )
{
	if( !factory )
	{ throw vl::null_pointer( "vl::cl::SceneManager::addMovableObjectFactory" ); }

	std::map<std::string, vl::graph::MovableObjectFactoryPtr>::iterator iter;
	iter = _movable_factories.find( factory->typeName() );
	if( iter != _movable_factories.end() )
	{ 
		// we either overwrite or it's an error
		if( overwrite )
		{ iter->second = factory; }
		else
		{
			throw vl::exception( "Factory overwrite not requested.", 
					"vl::cl::SceneManager::addMovableObjectFactory" );
		}
	}
	else
	{
		_movable_factories.insert(
				std::make_pair( factory->typeName(), factory ) );
	}
}

void
vl::cl::SceneManager::removeMovableObjectFactory(
		vl::graph::MovableObjectFactoryPtr factory )
{
	if( !factory )
	{ throw vl::null_pointer( "vl::cl::SceneManager::removeMovableObjectFactory" ); }

	removeMovableObjectFactory( factory->typeName() );
}

void
vl::cl::SceneManager::removeMovableObjectFactory( std::string const &typeName )
{
	std::map<std::string, vl::graph::MovableObjectFactoryPtr>::iterator iter;
	iter = _movable_factories.find( typeName );
	if( iter != _movable_factories.end() )
	{
		_movable_factories.erase( iter );
	}
	else
	{
		throw vl::exception( "No such factory",
					"vl::cl::SceneManager::addMovableObjectFactory" );
	}
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
