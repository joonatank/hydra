#include "eq_scene_node.hpp"

#include "eq_scene_manager.hpp"

#include <eq/net/session.h>

// SceneNode
vl::cl::SceneNode::SceneNode( vl::graph::SceneManagerRefPtr creator,
		std::string const &name )
	: _manager(creator),
	  _position(vl::vector::ZERO),
	  _rotation(vl::quaternion::IDENTITY),
	  _scale(1.0, 1.0, 1.0),
	  _attached(),
	  _parent(),
	  _childs()
{
	if( !creator )
	{ throw vl::null_pointer("SceneNode::SceneNode"); }

	// TODO add random name generator
	if( !name.empty() )
	{ eq::Object::setName( name ); }
}

vl::cl::SceneNode::~SceneNode( void )
{
	if( isMaster() )
	{ getSession()->deregisterObject( this ); }
	else
	{ getSession()->unmapObject( this ); }
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
vl::cl::SceneNode::attachObject( vl::graph::MovableObjectRefPtr object )
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
vl::cl::SceneNode::detachObject( vl::graph::MovableObjectRefPtr object )
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

vl::graph::SceneNodeRefPtr
vl::cl::SceneNode::createChild( std::string const &name )
{
	// TODO we should use the SceneManager to allocate the Nodes
	// and manage a list of created nodes.
	vl::graph::SceneNodeRefPtr child = _manager.lock()->createNode( name );
	addChild( child );

	return child;
}

// This function is only for internal usage, addChild and removeChild
// handles the user interface (we would introduce circular thinking by
// providing this function).
// Without moving the internal implementation to protected.
void
vl::cl::SceneNode::setParent( vl::graph::SceneNodeRefPtr parent )
{
	if( parent == shared_from_this() )
	{ throw vl::duplicate( "vl::cl::SceneNode::setParent" ); }

	//if( !parent )
	//{ throw vl::null_pointer( "vl::cl::SceneNode::setParent" ); }

	if( _parent.lock() == parent )
	{ return; }
	// Removed because throwing seems excessive.
	//{ throw vl::duplicate( "vl::cl::SceneNode::setParent" ); }

	// We have to change the parent here, so that removeChild knows
	// that it's called by setParent.
	vl::graph::SceneNodeRefPtr old_parent = _parent.lock();
	_parent = parent;

	old_parent->removeChild( shared_from_this() );
	
	// We allow NULL parents, only add childs if the parent is not NULL
	if( parent )
	{ parent->addChild( shared_from_this() ); }

	//((SceneNode *)parent->get())->_addChild( this );
	//_setParent( parent );
}

void
vl::cl::SceneNode::addChild( vl::graph::SceneNodeRefPtr child )
{
	if( !child )
	{ throw vl::null_pointer( "vl::cl::SceneNode::addChild" ); }

	if( shared_from_this() == child )
	{ throw vl::duplicate( "vl::cl::SceneNode::addChild" ); }

	// Parent has been already set, break the recursion
	if( child->getParent() == shared_from_this() )
	{
		_childs.push_back( child );
	}
	else
	{
		child->setParent( shared_from_this() );
	}

//	_addChild( child );
//	((SceneNode *)child->get())->_setParent( this );
}

vl::graph::SceneNodeRefPtr
vl::cl::SceneNode::getChild( uint16_t index )
{
	return _childs.at(index);
}

vl::graph::SceneNodeRefPtr
vl::cl::SceneNode::getChild( std::string const &name )
{
	vl::graph::SceneNodeRefPtr child = _findChild( name );
	return child;
}

void
vl::cl::SceneNode::removeChild( vl::graph::SceneNodeRefPtr child )
{
	if( shared_from_this() == child )
	{ throw vl::duplicate( "vl::cl::SceneNode::removeChild" ); }

	if( !child )
	{ throw vl::no_object( "vl::cl::SceneNode::removeChild" ); }

	// New parent is already set so called from setParent, break the recursion.
	if( child->getParent() != shared_from_this() )
	{
		vl::graph::ChildContainer::iterator iter =
			std::find( _childs.begin(), _childs.end(), child );
		if( iter != _childs.end() )
		{
			_childs.erase( iter );
		}
		// Throw as we assume that such child exists
		else
		{ throw vl::no_object( "vl::cl::SceneNode::removeChild" ); }
	}
	// Called from user space
	else
	{ child->setParent( shared_from_this() ); }
}

vl::graph::SceneNodeRefPtr
vl::cl::SceneNode::removeChild( uint16_t index )
{
	vl::graph::SceneNodeRefPtr child = _childs.at(index);
	//vl::cl::SceneNode *child = ptr.get();
	if( child )
	{
		removeChild( child );
		/*
		child->_parent.reset();
		vl::graph::ChildContainer::iterator iter =
			std::find( _childs.begin(), _childs.end(), child );
		if( iter != _childs.end() )
		{ _childs.erase( iter ); }
		*/

	//	_removeChild( child );
	//	child->_parent = 0;
	}
	return child;
}

vl::graph::SceneNodeRefPtr
vl::cl::SceneNode::removeChild( std::string const &name )
{
	vl::graph::SceneNodeRefPtr child = _findChild( name );
	if( child )
	{ removeChild( child ); }
	//vl::graph::ChildContainer::iterator iter = _findChild(name);
	//vl::graph::SceneNodeRefPtr child = *iter;
	//if( child )
	/*
	if( iter != _childs.end() )
	{
		(*iter)->_parent.reset();
		//vl::graph::ChildContainer::iterator iter =
		//	std::find( _childs.begin(), _childs.end(), child );
		//if( iter != _childs.end() )
		_childs.erase( iter );

		//_removeChild( child );
		//child->_parent = 0;
	}
	*/
	return child;
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
/*
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
*/

vl::graph::SceneNodeRefPtr
vl::cl::SceneNode::_findChild( std::string const &name )
{
	vl::graph::ChildContainer::iterator iter = _childs.begin();
	for( ; iter != _childs.end(); ++iter )
	{
		if( (*iter)->getName() == name )
		{
			return (*iter);
		}
	}

	return vl::graph::SceneNodeRefPtr();
}

vl::graph::ChildContainer::iterator 
vl::cl::SceneNode::_findChildIter( std::string const &name )
{
	vl::graph::ChildContainer::iterator iter = _childs.begin();
	for( ; iter != _childs.end(); ++iter )
	{
		if( (*iter)->getName() == name )
		{
			return iter;
		}
	}

	return _childs.end();
}
