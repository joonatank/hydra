#include "eq_scene_manager.hpp"

#include <eq/net/session.h>

// Functors
vl::cl::CreateNodeFunc::CreateNodeFunc( vl::cl::SceneManager *sm )
	: vl::cl::SceneFunctor<vl::cl::SceneNode *>(sm)
{}

vl::cl::SceneNode *
vl::cl::CreateNodeFunc::operator()( uint32_t const &id )
{
	EQASSERT( _manager );

	SceneNode *node = (vl::cl::SceneNode *)_manager->getNode( id );
	EQASSERT( node );
	return node;
}

vl::cl::DeleteNodeFunc::DeleteNodeFunc( vl::cl::SceneManager *sm )
	: vl::cl::SceneFunctor<vl::cl::SceneNode *>(sm)
{}

vl::cl::SceneNode *
vl::cl::DeleteNodeFunc::operator()( uint32_t const &id )
{
	EQASSERT( _manager );

	SceneNode *node = (vl::cl::SceneNode *)_manager->getNode( id );
	EQASSERT( node );
	return node;
}

vl::cl::CreateObjectFunc::CreateObjectFunc( vl::cl::SceneManager *sm )
	: vl::cl::SceneFunctor<vl::cl::MovableObject *>(sm)
{}

vl::cl::MovableObject *
vl::cl::CreateObjectFunc::operator()( uint32_t const &id )
{
	EQASSERT( _manager );

	MovableObject *obj = (vl::cl::MovableObject *)_manager->getObject( id );
	EQASSERT( obj );
	return obj;
}

vl::cl::DeleteObjectFunc::DeleteObjectFunc( vl::cl::SceneManager *sm )
	: vl::cl::SceneFunctor<vl::cl::MovableObject *>(sm)
{}

vl::cl::MovableObject *
vl::cl::DeleteObjectFunc::operator()( uint32_t const &id )
{
	EQASSERT( _manager );

	MovableObject *obj = (vl::cl::MovableObject *)_manager->getObject( id );
	EQASSERT( obj );
	return obj;
}

// ENDOF Functors

// SceneManager
vl::cl::SceneManager::SceneManager( std::string const &name )
	: _nodes(0), _objects(0), _objectCreateFunc(0), 
	  _objectDeleteFunc(0), _nodeCreateFunc(0), _nodeDeleteFunc(0), _root(0)
{
	eq::Object::setName( name );
//	_createDistribContainers( );
	// This is problematic as if we call this it will call
	// register object, so equalizer has to be running in
	// main loop already.
//	_root = createNodeImpl( "Root" );
}

vl::graph::SceneNode *
vl::cl::SceneManager::createNodeImpl( std::string const &name )
{
	vl::cl::SceneNode *node = _createSceneNodeImpl( name );

	// TODO add to nodes, we should also share it here so there
	// will be ID generated for this node
	// Seems not to need to be registered.
	// Um the ID is invalid so we need to register the scene_manager first
	// or register the objects later.
	getSession()->registerObject( node );
	std::cout << "Creating scene node = " << node->getName()
		<< " with ID = " << node->getID() << std::endl;
	std::cout << "node ID ==  EQ_ID_INVALID = "
		<< (node->getID() == EQ_ID_INVALID) << std::endl;
	// This needs the ID to be generated already so we need to
	// register the node first.
	_nodes->push( node );
	_nodes->commit();
	return node;
}

vl::graph::MovableObject *
vl::cl::SceneManager::createEntity(
		std::string const &name, std::string const &meshName )
{
	vl::NamedValuePairList params;
	params["meshName"] = meshName;
	MovableObject *obj = _createMovableObjectImpl(
			"Entity", name, params );
	_objects->push( obj );

	return obj;
}

vl::graph::SceneNode *
vl::cl::SceneManager::getNode( std::string const &name )
{
	std::cout << "SceneManager::getNode : " << name << std::endl;
	std::cout << "size = " << _nodes->size() << std::endl;
	for( size_t i = 0; i < _nodes->size(); ++i )
	{
		std::cout << "node name = " << _nodes->at(i)->getName() << std::endl;
		if( _nodes->at(i)->getName() == name )
		{
			return _nodes->at(i);
		}
	}

	return 0;
}

vl::graph::SceneNode *
vl::cl::SceneManager::getNode( uint32_t id )
{
	for( size_t i = 0; i < _nodes->size(); i++ )
	{
		if( _nodes->at(i)->getID() == id )
		{ return _nodes->at(i); }
	}

	return 0;
}

vl::graph::MovableObject *
vl::cl::SceneManager::getObject( uint32_t id )
{
	for( size_t i = 0; i < _objects->size(); i++ )
	{
		if( _objects->at(i)->getID() == id )
		{ return _objects->at(i); }
	}

	return 0;
}

// equalizer overrides
void
vl::cl::SceneManager::serialize( eq::net::DataOStream& os,
		const uint64_t dirtyBits )
{
	if( DIRTY_OBJECTS & dirtyBits )
	{
		if ( _objects )
		{
			std::cout << "Registering object container" << std::endl;
			//getSession()->registerObject( _objects );
			os << _objects->getID();
		}
		else
		{ os << EQ_ID_INVALID; }
	}
	if( DIRTY_NODES & dirtyBits )
	{
		if( _nodes )
		{
			std::cout << "Registering node container" << std::endl;
			// We can't register objects here, because this is called by
			// get getInstanceData() which is called by registerObject()
			//getSession()->registerObject( _nodes );
			os << _nodes->getID();
		}
		else
		{ os << EQ_ID_INVALID; }
	}
}

void
vl::cl::SceneManager::deserialize( eq::net::DataIStream& is,
						  const uint64_t dirtyBits )
{
	if( DIRTY_OBJECTS & dirtyBits )
	{
		uint32_t id;
		is >> id;
		if( id != EQ_ID_INVALID )
		{
			std::cout << "mapping objects container" << std::endl;
			if( _objects )
			{ getSession()->mapObject( _objects, id ); }
			else
			{ std::cerr << "no _objects to map" << std::endl; }
		}
	}
	if( DIRTY_NODES & dirtyBits )
	{
		uint32_t id;
		is >> id;
		if( id != EQ_ID_INVALID )
		{
			std::cout << "mapping nodes container" << std::endl;
			if( _nodes )
			{ getSession()->mapObject( _nodes, id ); }
			else
			{ std::cerr << "no _nodes to map" << std::endl; }
		}
	}
}

void
vl::cl::SceneManager::finalize( void )
{
	std::cout << "SceneManager::finalize" << std::endl;
	// TODO we need to check that we are registered here
	_createDistribContainers( );
	if( isMaster() )
	{
		getSession()->registerObject( _objects );
		setDirty( DIRTY_OBJECTS );
		getSession()->registerObject( _nodes );
		setDirty( DIRTY_NODES );
	}
}

vl::cl::SceneNode *
vl::cl::SceneManager::_createSceneNodeImpl( std::string const &name )
{
	return new vl::cl::SceneNode( this, name );
}

vl::cl::MovableObject *
vl::cl::SceneManager::_createMovableObjectImpl(
		std::string const &typeName, std::string const &name,
		vl::NamedValuePairList const &params )
{
	// For now we only use entities
	if( typeName != "Entity" )
	{ return 0; }

	return new vl::cl::Entity( name, params );
}

void
vl::cl::SceneManager::_createDistribContainers( void )
{
	_objectCreateFunc = new CreateObjectFunc( this );
	_objectDeleteFunc = new DeleteObjectFunc( this );

	_objects = new DistributedContainer<MovableObject *>(
			_objectCreateFunc, _objectDeleteFunc );

	_nodeCreateFunc = new CreateNodeFunc( this );
	_nodeDeleteFunc = new DeleteNodeFunc( this );

	_nodes = new DistributedContainer<SceneNode *>(
			_nodeCreateFunc, _nodeDeleteFunc );
}
