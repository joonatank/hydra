/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-04
 *	@file movable_object.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

#include "movable_object.hpp"

#include "scene_node.hpp"

#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreMovableObject.h>

vl::MovableObject::MovableObject(std::string const &name, vl::SceneManagerPtr creator, bool dynamic)
	: _name(name)
	, _visible(true)
	, _creator(creator)
	, _parent(0)
	, _listener(0)
	, _orientation(Ogre::Quaternion::IDENTITY)
	, _position(Ogre::Vector3::ZERO)
	, _is_dynamic(dynamic)
{}

vl::MovableObject::MovableObject(vl::SceneManagerPtr creator)
	: _name(), _visible(true), _creator(creator), _parent(0)
	, _listener(0), _is_dynamic(false)
{}

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

void 
vl::MovableObject::setVisible(bool visible)
{
	update_variable(_visible, visible, DIRTY_VISIBLE);
}


void 
vl::MovableObject::setPosition(Ogre::Vector3 const &pos)
{
	if( _position != pos )
	{
		setDirty(DIRTY_TRANSFORMATION);
		_position = pos;
	}
}

void 
vl::MovableObject::setOrientation(Ogre::Quaternion const &q)
{
	if( _orientation != q )
	{
		setDirty(DIRTY_TRANSFORMATION);
		_orientation = q;
	}
}


Ogre::Quaternion
vl::MovableObject::getWorldOrientation(void) const	
{
	if(_parent)
	{ return _parent->getWorldTransform()*_orientation; }
	else
	{ return _orientation; }
}

Ogre::Vector3
vl::MovableObject::getWorldPosition(void) const
{
	if(_parent)
	{ return _parent->getWorldTransform()*_position; }
	else
	{ return _position; }
}



/// -------------------------------------- Private ---------------------------
bool 
vl::MovableObject::_createNative(void)
{
	bool ret = _doCreateNative();

	if( _parent )
	{ setParent(_parent); }

	return ret;
}

void
vl::MovableObject::serialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits ) const
{
	if( DIRTY_NAME & dirtyBits )
	{
		msg << _name;
	}

	if( DIRTY_VISIBLE & dirtyBits )
	{
		msg << _visible;
	}

	if(DIRTY_TRANSFORMATION & dirtyBits)
	{
		msg << _position << _orientation;
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

	if( DIRTY_VISIBLE & dirtyBits )
	{
		msg >> _visible;
		if(getNative())
		{ getNative()->setVisible(_visible); }
	}

	if(DIRTY_TRANSFORMATION & dirtyBits)
	{
		msg >> _position >> _orientation;
		// Ogre objects may or may not have transformation so we use
		// virtual callback
		_transformation_updated();
	}

	doDeserialize(msg, dirtyBits);
		
	if( recreate )
	{
		_createNative();

		/// For background loaded they don't have natives
		if(getNative())
		{ getNative()->setVisible(_visible); }

		// Possible to avoid rendering them to the window directly
		if(getNative())
		{
			// When using Deferred shading lights need to be in the second pass
			// while other objects are in the first.
			// For other renderers we render both in the first pass.
			if(getTypeName() == "Light")
			{ getNative()->setVisibilityFlags(1<<1); }
			else
			{ getNative()->setVisibilityFlags(1); }
		}

		_transformation_updated();
	}
}
