/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-01
 *	@file physics/motion_state.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef HYDRA_PHYSICS_MOTION_STATE_HPP
#define HYDRA_PHYSICS_MOTION_STATE_HPP

// This class initialises Bullet physics so they are necessary
#include <bullet/btBulletDynamicsCommon.h>

#include "math/math.hpp"
#include "math/conversion.hpp"

#include "scene_node.hpp"

namespace vl
{

namespace physics
{

/** @class MotionState
 *
 */
class MotionState : public btMotionState
{
public:
	MotionState( Ogre::Vector3 const &pos,
				 Ogre::Quaternion const &orient,
				 vl::SceneNode *node = 0)
		: _visibleobj(node)
	{
		_trans = vl::math::convert_bt_transform(orient, pos);
	}

	MotionState(vl::Transform const &trans, vl::SceneNode *node = 0)
		: _visibleobj(node)
	{
		_trans = vl::math::convert_bt_transform(trans);
		// set the object transform especially useful for static objects
		if(_visibleobj)
		{
			_visibleobj->setPosition(trans.position);
			_visibleobj->setOrientation(trans.quaternion);
		}
	}

	virtual ~MotionState()
	{}

	vl::SceneNodePtr getNode(void) const
	{ return _visibleobj; }

	void setNode(vl::SceneNodePtr node)
	{
		_visibleobj = node;
		// set the object transform especially useful for static objects
		if(_visibleobj)
		{
			/// @todo these should set the world transform
			Transform trans = vl::math::convert_transform(_trans);
			_visibleobj->setPosition(trans.position);
			_visibleobj->setOrientation(trans.quaternion);
		}
	}

	Ogre::Vector3 getPosition(void) const
	{ return vl::math::convert_vec(_trans.getOrigin()); }

	void setPosition(Ogre::Vector3 const &v)
	{ _trans.setOrigin(vl::math::convert_bt_vec(v)); }

	Ogre::Quaternion getOrientation(void) const
	{ return vl::math::convert_quat(_trans.getRotation()); }

	void setOrientation(Ogre::Quaternion const &q)
	{ _trans.setRotation(vl::math::convert_bt_quat(q)); }

	Transform getWorldTransform(void) const
	{
		return vl::math::convert_transform(_trans);
	}

	void setWorldTransform(vl::Transform const &worldTrans)
	{
		_trans = vl::math::convert_bt_transform(worldTrans);
	}

	virtual void getWorldTransform(btTransform &worldTrans) const
	{
		worldTrans = _trans;
	}

	virtual void setWorldTransform(const btTransform &worldTrans)
	{
		_trans = worldTrans;

		// silently return before we set a node
		if(_visibleobj)
		{ _visibleobj->setWorldTransform(vl::math::convert_transform(_trans)); }
	}

protected:
	SceneNodePtr _visibleobj;
	btTransform _trans;

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
