/**
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-01
 *	@file physics/motion_state.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

#ifndef HYDRA_PHYSICS_MOTION_STATE_HPP
#define HYDRA_PHYSICS_MOTION_STATE_HPP

#include "math/math.hpp"
#include "math/transform.hpp"
// Necessary for HYDRA_API
#include "defines.hpp"

#include "object_interface.hpp"

namespace vl
{

namespace physics
{

/** @class MotionState
 *
 */
class HYDRA_API MotionState
{
public:
	MotionState(Transform const &t, vl::ObjectInterface *node = 0)
		: _visibleobj(node)
		, _trans(t)
	{
		// set the object transform especially useful for static objects
		if(_visibleobj)
		{
			_visibleobj->setWorldTransform(_trans);
		}
	}

	static MotionState *create(Transform const &t, vl::ObjectInterface *node);

	virtual ~MotionState()
	{}

	vl::ObjectInterface *getNode(void) const
	{ return _visibleobj; }

	void setNode(vl::ObjectInterface *node)
	{
		_visibleobj = node;
		// set the object transform especially useful for static objects
		if(_visibleobj)
		{
			_visibleobj->setWorldTransform(_trans);
		}
	}

	Ogre::Vector3 const &getPosition(void) const
	{ return _trans.position; }

	void setPosition(Ogre::Vector3 const &v)
	{ _trans.position = v; }

	Ogre::Quaternion const &getOrientation(void) const
	{ return _trans.quaternion; }

	void setOrientation(Ogre::Quaternion const &q)
	{ _trans.quaternion = q; }

	Transform const &getWorldTransform(void) const
	{
		return _trans;
	}

	void setWorldTransform(vl::Transform const &worldTrans)
	{
		_trans = worldTrans;

		// silently return before we set a node
		if(_visibleobj)
		{ _visibleobj->setWorldTransform(_trans); }
	}

protected:
	vl::ObjectInterface *_visibleobj;
	vl::Transform _trans;

};	// class MotionState

inline std::ostream &
operator<<(std::ostream &os, MotionState const &m)
{
	os << "Transform : ";
	if( m.getNode() )
	{ os << "with SceneNode " << m.getNode()->getName(); }
	else
	{ os << "without SceneNode"; }
	os << " : position = " << m.getPosition() << " : orientation = " << m.getOrientation(); 

	return os;
}

}	// namespace physics

}	// namespace vl

#endif // HYDRA_PHYSICS_MOTION_STATE_HPP
