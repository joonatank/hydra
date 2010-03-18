#include "eq_scene_node.hpp"

#include "eq_scene_manager.hpp"

#include <eq/net/session.h>

// SceneNode
vl::cl::SceneNode::SceneNode( vl::graph::SceneManager *creator,
		std::string const &name )
	: _creator(creator),
	  _position(vl::vector::ZERO),
	  _rotation(vl::quaternion::IDENTITY),
	  _scale(1.0, 1.0, 1.0),
	  _attached(),
	  _parent(0),
	  _childs()
{
	if( !_creator )
	{ throw vl::null_pointer("SceneNode::SceneNode"); }

	if( !name.empty() )
	{ eq::Object::setName( name ); }
}

void
vl::cl::SceneNode::translate( vl::vector const &v,
		TransformSpace relativeTo )
{
	if( v != vl::vector::ZERO )
	{
		setDirty( DIRTY_TRANSFORM );
		_position += v;
	}
}

void
vl::cl::SceneNode::setPosition( vl::vector const &v,
		TransformSpace relativeTo )
{
	if( v != _position )
	{
		setDirty( DIRTY_TRANSFORM );
		_position = v;
	}
}

void
vl::cl::SceneNode::rotate( vl::quaternion const &q,
		TransformSpace relativeTo )
{
	if( !vl::equal( q.abs(), 1.0 ) )
	{ throw vl::scale_quaternion("SceneNode::rotate"); }

	if( !vl::equal(q, vl::quaternion::IDENTITY) )
	{
		setDirty( DIRTY_TRANSFORM );
		_rotation *= q;
	}
}

void
vl::cl::SceneNode::setOrientation( vl::quaternion const &q,
		TransformSpace relativeTo )
{
	if( !vl::equal( q.abs(), 1.0 ) )
	{ throw vl::scale_quaternion("SceneNode::setOrientation"); }

	if( !vl::equal(q, _rotation) )
	{
		setDirty( DIRTY_TRANSFORM );
		_rotation = q;
	}
}

void
vl::cl::SceneNode::scale( vl::vector const &s )
{
	if( vl::equal(s, vl::vector::ZERO) )
	{ throw vl::zero_scale("scene_node::scale"); }

	if( !vl::equal(s, vl::vector( 1, 1, 1 )) )
	{
		setDirty( DIRTY_SCALE );
		_scale *= s;
	}
}

void
vl::cl::SceneNode::scale( vl::scalar const s )
{
	if( vl::equal(s, 0.0) )
	{ throw vl::zero_scale("scene_node::scale"); }

	if( !vl::equal(s, 1.0) )
	{
		setDirty( DIRTY_SCALE );
		_scale *= s;
	}
}

void 
vl::cl::SceneNode::setScale( vl::vector const &s )
{
	if( vl::equal(s, vl::vector::ZERO) )
	{ throw vl::zero_scale("scene_node::scale"); }

	if( !vl::equal(s, _scale) )
	{
		setDirty( DIRTY_SCALE );
		_scale = s;
	}
}

void
vl::cl::SceneNode::attachObject( vl::graph::MovableObject *object )
{
	vl::graph::ObjectContainer::iterator iter = _attached.begin();
	for( ; iter != _attached.end(); ++iter )
	{
		if( *iter == object )
		{ throw vl::duplicate( "vl::cl::SceneNode::attachObject" ); }
	}

	_attached.push_back( object );
}

void
vl::cl::SceneNode::detachObject( vl::graph::MovableObject *object )
{
	vl::graph::ObjectContainer::iterator iter = _attached.begin();
	for( ; iter != _attached.end(); ++iter )
	{
		if( *iter == object )
		{
			_attached.erase( iter );
			return;
		}
	}

	// TODO we have a problem with allocation and deallocation
	// here, so we just let application programmer to
	// destroy the object... RefPtrs would work better.
	//
	// Should movableObjects be owned by only one node or not?
}

vl::graph::SceneNode *
vl::cl::SceneNode::createChild( std::string const &name )
{
	// TODO we should use the SceneManager to allocate the Nodes
	// and manage a list of created nodes.
	vl::graph::SceneNode *child = _creator->createNode( name );
	addChild( child );

	return child;
}

// This function is only for internal usage, addChild and removeChild
// handles the user interface (we would introduce circular thinking by
// providing this function).
// Without moving the internal implementation to protected.
void
vl::cl::SceneNode::setParent( vl::graph::SceneNode *parent )
{
	// TODO should throw
	if( (vl::graph::SceneNode *)this == parent )
	{
		throw vl::duplicate( //"Can not be child and parent",
			"vl::cl::SceneNode::setParent" );
	}

	if( 0 == parent )
	{ throw vl::null_pointer( "vl::cl::SceneNode::setParent" ); }

	if( _parent == parent )
	{
		throw vl::duplicate( "vl::cl::SceneNode::setParent" );
	}

	((SceneNode *)parent)->_addChild( this );
	_setParent( parent );
}

void
vl::cl::SceneNode::addChild( vl::graph::SceneNode *child )
{
	if( 0 == child )
	{ throw vl::null_pointer( "vl::cl::SceneNode::addChild" ); }

	if( this == child )
	{
		throw vl::duplicate( //"Can not be child and parent",
			"vl::cl::SceneNode::addChild" );
	}

	_addChild( child );
	((SceneNode *)child)->_setParent( this );
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

// ------- Protected ---------
void
vl::cl::SceneNode::_addChild( vl::graph::SceneNode *child )
{
	vl::graph::ChildContainer::iterator iter = _childs.begin();
	for( ; iter != _childs.end(); ++iter )
	{
		if( *iter == child )
		{ throw vl::duplicate("vl::cl::SceneNode::_addChild"); }
	}

	_childs.push_back( child );
}

void
vl::cl::SceneNode::_setParent( vl::graph::SceneNode *parent )
{
	if( _parent )
	{ ((SceneNode *)_parent)->_removeChild( this ); }

	_parent = parent;
}

void
vl::cl::SceneNode::_removeChild( vl::graph::SceneNode *child )
{
	vl::graph::ChildContainer::iterator iter = _childs.begin();
	for( ; iter != _childs.end(); ++iter )
	{
		if( *iter == child )
		{
			_childs.erase( iter );
			return;
		}
	}

	throw vl::exception("Child not found", "vl::cl::SceneNode::_removeChild" );
}
