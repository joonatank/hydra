
#include "movable_object.hpp"

#include "scene_node.hpp"

#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreMovableObject.h>

void 
vl::MovableObject::setParent(vl::SceneNodePtr parent)
{
	_parent = parent;
	if( getNative() && _parent->getNative() )
	{
		if( getNative()->isAttached() )
		{
			if( getNative()->getParentSceneNode() != _parent->getNative() )
			{
				getNative()->detachFromParent();
				_parent->getNative()->attachObject(getNative());
			}
		}
		else
		{
			_parent->getNative()->attachObject(getNative());
		}
	}
}

bool 
vl::MovableObject::_createNative(void)
{
	bool ret = _doCreateNative();

	if( _parent )
	{ setParent(_parent); }

	return ret;
}

void
vl::MovableObject::serialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits )
{
	if( DIRTY_NAME & dirtyBits )
	{
		msg << _name;
	}

	doSerialize(msg, dirtyBits);
}

void 
vl::MovableObject::deserialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits)
{
	bool recreate = false;

	if( DIRTY_NAME & dirtyBits )
	{
		msg >> _name;
		recreate = true;
	}

	doDeserialize(msg, dirtyBits);
		
	if( recreate )
	{ _createNative(); }
}
