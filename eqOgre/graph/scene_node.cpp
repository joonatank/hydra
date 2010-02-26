#include "scene_node.hpp"

#include "scene_manager.hpp"

#include <eq/net/session.h>

// Functors
vl::graph::AttachNodeFunc::AttachNodeFunc( vl::graph::SceneManager *sm,
		vl::graph::SceneNode *owner )
	: vl::graph::SceneFunctor<SceneNode *>(sm),
	  _owner( owner )
{}

vl::graph::SceneNode *
vl::graph::AttachNodeFunc::operator()( uint32_t const &id )
{
	SceneNode *node = _manager->getNode( id );
	if( node )
	{
		// Call the protected function which handles
		// the lower level engine specific functionality
		if( _owner )
		{ _owner->_addChild( node); }
	}
	else
	{
		// Should throw
	}
	return node;
}

vl::graph::DetachNodeFunc::DetachNodeFunc( SceneManager *sm, SceneNode *owner)
	: SceneFunctor<SceneNode *>(sm),
	  _owner( owner )
{}

vl::graph::SceneNode *
vl::graph::DetachNodeFunc::operator()( uint32_t const &id )
{
	EQASSERT( id != EQ_ID_INVALID );

	// This should remove the node from the owners list of
	// childs
	SceneNode *node= _manager->getNode( id );
	if( node )
	{
		// Call the protected function which handles
		// the lower level engine specific functionality
		if( _owner )
		{ _owner->_removeChild( node ); }
	}
	else
	{
		// Should throw
	}
	return node;
}

vl::graph::AttachObjectFunc::AttachObjectFunc( vl::graph::SceneManager *sm,
		vl::graph::SceneNode *owner )
	: vl::graph::SceneFunctor<vl::graph::MovableObject *>(sm),
	  _owner( owner )
{}

vl::graph::MovableObject *
vl::graph::AttachObjectFunc::operator()( uint32_t const &id )
{
	MovableObject *obj = _manager->getObject( id );
	if( obj )
	{
		// Call the protected function which handles
		// the lower level engine specific functionality
		if( _owner )
		{ _owner->_attachObject( obj ); }
	}
	else
	{
		// Should throw
	}
	return obj;
}

vl::graph::DetachObjectFunc::DetachObjectFunc( vl::graph::SceneManager *sm,
		vl::graph::SceneNode *owner )
	: vl::graph::SceneFunctor<MovableObject *>(sm),
	  _owner( owner )
{}

vl::graph::MovableObject *
vl::graph::DetachObjectFunc::operator()( uint32_t const &id )
{
	MovableObject *obj = _manager->getObject( id );
	if( obj )
	{
		// Call the protected function which handles
		// the lower level engine specific functionality
		_owner->_detachObject( obj );
	}
	else
	{
		// Should throw
	}
	return obj;
}

// ENDOF Functors

// SceneNode
vl::graph::SceneNode::SceneNode( vl::graph::SceneManager *creator,
		std::string const &name )
	: _creator(creator),
	  _childDetachFunc( creator, this ),
	  _childAttachFunc( creator, this ),
	  _objectDetachFunc( creator, this ),
	  _objectAttachFunc( creator, this ),
	  _attached(&_objectAttachFunc,  &_objectDetachFunc),
	  _childs(&_childAttachFunc, &_childDetachFunc)
{
	if( !name.empty() )
	{ eq::Object::setName( name ); }
}

void
vl::graph::SceneNode::destroy( void )
{
	for( size_t i = 0; i < _attached.size(); ++i )
	{ _creator->destroy( _attached.at(i) ); }

	for( size_t i = 0; i < _childs.size(); ++i )
	{ _creator->destroy( _childs.at(i) ); }

	_creator->destroy( this );
}
void
vl::graph::SceneNode::attachObject( vl::graph::MovableObject *object )
{
	bool found = false;
	for( size_t i = 0; i < _attached.size(); i++ )
	{
		if( object == _attached.at(i) )
		{ found = true; break; }
	}
	if( !found )
	{
	//	setDirty( DIRTY_ATTACHED );

		_attached.push( object );
	}
}

void
vl::graph::SceneNode::detachObject( vl::graph::MovableObject *object )
{
	for( size_t i = 0; i < _attached.size(); i++ )
	{
		if( object == _attached.at(i) )
		{
			_attached.remove( i );
		}
	}
		// TODO we have a problem with allocation and deallocation
		// here, so we just let application programmer to
		// destroy the object... RefPtrs would work better.
		//
		// Should movableObjects be owned by only one node or not?
}

vl::graph::SceneNode *
vl::graph::SceneNode::createChild( std::string const &name )
{
	// TODO we should use the SceneManager to allocate the Nodes
	// and manage a list of created nodes.
	vl::graph::SceneNode *child = _creator->createNodeImpl( name );
	addChild( child );

	return child;
}

// This function is only for internal usage, addChild and removeChild
// handles the user interface (we would introduce circular thinking by
// providing this function).
// Without moving the internal implementation to protected.
void
vl::graph::SceneNode::setParent( vl::graph::SceneNode *parent )
{
	// TODO should throw
	if( this == parent )
	{ return; }

	// We need to inform our current owner of the transfer
	// and we need to inform our new owner after that.
	_parent = parent;
}

void
vl::graph::SceneNode::addChild( vl::graph::SceneNode *child )
{
	// TODO should throw
	if( !child || this == child || child->parent() == this )
	{ return; }

	// Set dirty
//	setDirty( DIRTY_CHILD );

	for( size_t i = 0; i < _childs.size(); i++ )
	{
		if( _childs.at(i) == child )
		{
			// TODO this should throw, as the parent and child are
			// inconsistent
			return;
		}
	}
	child->setParent( this );
	_childs.push( child );
}


// ---- Equalizer overrides ----
void
vl::graph::SceneNode::serialize( eq::net::DataOStream& os,
		const uint64_t dirtyBits )
{
	eq::Object::serialize( os, dirtyBits );
	
	if( dirtyBits & DIRTY_TRANSFORM )
	{ os << _position << _rotation; }

	if( dirtyBits & DIRTY_SCALE )
	{ os << _scale; }

	if( dirtyBits & DIRTY_ATTACHED )
	{
		// TODO this should register _attached as eq distributed object
		os << _attached.getID();
	}

	if( dirtyBits & DIRTY_CHILDS )
	{
		// TODO this should register _childs as eq distributed object
		os << _childs.getID();
	}
}

void
vl::graph::SceneNode::deserialize( eq::net::DataIStream& is,
		const uint64_t dirtyBits )
{
	eq::Object::deserialize( is, dirtyBits );
	
	if( dirtyBits & DIRTY_TRANSFORM )
	{
		is >> _position >> _rotation;
		_setTransform( _position, _rotation );
	}

	if( dirtyBits & DIRTY_SCALE )
	{
		is >> _scale;
		_setScale( _scale );
	}

	if( dirtyBits & DIRTY_ATTACHED )
	{
		uint32_t id;
		is >> id;
		// TODO this should map _attached to eq distributed object
		getSession()->mapObject( &_attached, id );
	}

	if( dirtyBits & DIRTY_CHILDS )
	{
		uint32_t id;
		is >> id;
		// TODO this should map _childs to eq distributed object
		getSession()->mapObject( &_childs, id );
	}

}
