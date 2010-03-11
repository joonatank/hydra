#include "eq_scene_node.hpp"

#include "eq_scene_manager.hpp"

#include <eq/net/session.h>

// Functors
/*
vl::cl::AttachNodeFunc::AttachNodeFunc( vl::cl::SceneManager *sm,
		vl::cl::SceneNode *owner )
	: vl::cl::SceneFunctor<vl::cl::SceneNode *>(sm),
	  _owner( owner )
{}

vl::cl::SceneNode *
vl::cl::AttachNodeFunc::operator()( uint32_t const &id )
{
	SceneNode *node = (vl::cl::SceneNode *)_manager->getNode( id );
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

vl::cl::DetachNodeFunc::DetachNodeFunc( vl::cl::SceneManager *sm,
		vl::cl::SceneNode *owner)
	: SceneFunctor<vl::cl::SceneNode *>(sm),
	  _owner( owner )
{}

vl::cl::SceneNode *
vl::cl::DetachNodeFunc::operator()( uint32_t const &id )
{
	EQASSERT( id != EQ_ID_INVALID );

	// This should remove the node from the owners list of
	// childs
	SceneNode *node= (vl::cl::SceneNode *)_manager->getNode( id );
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

vl::cl::AttachObjectFunc::AttachObjectFunc( vl::cl::SceneManager *sm,
		vl::cl::SceneNode *owner )
	: vl::cl::SceneFunctor<vl::cl::MovableObject *>(sm),
	  _owner( owner )
{}

vl::cl::MovableObject *
vl::cl::AttachObjectFunc::operator()( uint32_t const &id )
{
	MovableObject *obj = (vl::cl::MovableObject *)_manager->getObject( id );
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

vl::cl::DetachObjectFunc::DetachObjectFunc( vl::cl::SceneManager *sm,
		vl::cl::SceneNode *owner )
	: vl::cl::SceneFunctor<vl::cl::MovableObject *>(sm),
	  _owner( owner )
{}

vl::cl::MovableObject *
vl::cl::DetachObjectFunc::operator()( uint32_t const &id )
{
	MovableObject *obj = (vl::cl::MovableObject *)_manager->getObject( id );
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
*/

// ENDOF Functors

// SceneNode
vl::cl::SceneNode::SceneNode( vl::graph::SceneManager *creator,
		std::string const &name )
	: _creator(creator),
	  _position(vl::vector::ZERO),
	  _rotation(vl::quaternion::IDENTITY),
	  _scale(1.0, 1.0, 1.0),
	  _attached(),
	  _childs()
	/*
	  _childDetachFunc( creator, this ),
	  _childAttachFunc( creator, this ),
	  _objectDetachFunc( creator, this ),
	  _objectAttachFunc( creator, this ),
	  _attached(&_objectAttachFunc,  &_objectDetachFunc),
	  _childs(&_childAttachFunc, &_childDetachFunc)
  */
{
	if( !_creator )
	{ throw vl::null_pointer("SceneNode::SceneNode"); }

	if( !name.empty() )
	{ eq::Object::setName( name ); }
}

void
vl::cl::SceneNode::destroy( void )
{
	for( size_t i = 0; i < _attached.size(); ++i )
	{ _creator->destroy( _attached.at(i) ); }

	for( size_t i = 0; i < _childs.size(); ++i )
	{ _creator->destroy( _childs.at(i) ); }

	_creator->destroy( this );
}
void
vl::cl::SceneNode::attachObject( vl::graph::MovableObject *object )
{
	/*
	vl::cl::MovableObject *cl_obj = (vl::cl::MovableObject *)object;
	bool found = false;
	for( size_t i = 0; i < _attached.size(); i++ )
	{
		if( cl_obj == _attached.at(i) )
		{ found = true; break; }
	}
	if( !found )
	{
	//	setDirty( DIRTY_ATTACHED );

		_attached.push( cl_obj );
	}
	*/
}

void
vl::cl::SceneNode::detachObject( vl::graph::MovableObject *object )
{
	/*
	vl::cl::MovableObject *cl_obj = (vl::cl::MovableObject *)object;
	for( size_t i = 0; i < _attached.size(); i++ )
	{
		if( cl_obj == _attached.at(i) )
		{
			_attached.remove( i );
		}
	}
	*/

		// TODO we have a problem with allocation and deallocation
		// here, so we just let application programmer to
		// destroy the object... RefPtrs would work better.
		//
		// Should movableObjects be owned by only one node or not?
}

vl::graph::SceneNode *
vl::cl::SceneNode::createChild( std::string const &name )
{
	/*
	// TODO we should use the SceneManager to allocate the Nodes
	// and manage a list of created nodes.
	vl::graph::SceneNode *child = _creator->createNodeImpl( name );
	addChild( child );

	return child;
	*/
	return 0;
}

// This function is only for internal usage, addChild and removeChild
// handles the user interface (we would introduce circular thinking by
// providing this function).
// Without moving the internal implementation to protected.
void
vl::cl::SceneNode::setParent( vl::graph::SceneNode *parent )
{
	/*
	// TODO should throw
	if( this == parent )
	{ return; }

	// We need to inform our current owner of the transfer
	// and we need to inform our new owner after that.
	_parent = parent;
	*/
}

void
vl::cl::SceneNode::addChild( vl::graph::SceneNode *child )
{
	/*
	vl::cl::SceneNode *cl_child = (vl::cl::SceneNode *)child;

	// TODO should throw
	if( !cl_child || this == cl_child || cl_child->parent() == this )
	{ return; }

	// Set dirty
//	setDirty( DIRTY_CHILD );

	for( size_t i = 0; i < _childs.size(); i++ )
	{
		if( _childs.at(i) == cl_child )
		{
			// TODO this should throw, as the parent and child are
			// inconsistent
			return;
		}
	}
	cl_child->setParent( this );
	_childs.push( cl_child );
	*/
}

void
vl::cl::SceneNode::removeChild( vl::graph::SceneNode *child )
{
	/*
	vl::cl::SceneNode *cl_child = (vl::cl::SceneNode *)child;
	for( size_t i = 0; i < _childs.size(); ++i )
	{
		if( _childs.at(i) == cl_child )
		{
			_childs.remove(i);
			cl_child->setParent( 0 );
			break;
		}
	}
	*/
}

// ---- Equalizer overrides ----
void
vl::cl::SceneNode::serialize( eq::net::DataOStream& os,
		const uint64_t dirtyBits )
{
	eq::Object::serialize( os, dirtyBits );
	
	if( dirtyBits & DIRTY_TRANSFORM )
	{ os << _position << _rotation; }

	if( dirtyBits & DIRTY_SCALE )
	{ os << _scale; }

	/*
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
	*/
}

void
vl::cl::SceneNode::deserialize( eq::net::DataIStream& is,
		const uint64_t dirtyBits )
{
	eq::Object::deserialize( is, dirtyBits );
	
	if( dirtyBits & DIRTY_TRANSFORM )
	{
		is >> _position >> _rotation;
	}

	if( dirtyBits & DIRTY_SCALE )
	{
		is >> _scale;
	}

	/*
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
	*/

}
