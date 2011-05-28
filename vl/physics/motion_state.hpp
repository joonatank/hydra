/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-01
 *	@file motion_state.hpp
 *
 *	This file is part of Hydra a VR game engine.
 */

#ifndef VL_PHYSICS_MOTION_STATE_HPP
#define VL_PHYSICS_MOTION_STATE_HPP

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

	MotionState( vl::Transform const &trans,
				 vl::SceneNode *node = 0)
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
			vl::Transform trans = vl::math::convert_transform(_trans);
			_visibleobj->setPosition(trans.position);
			_visibleobj->setOrientation(trans.quaternion);
		}
	}

	Ogre::Vector3 getPosition(void) const
	{ return vl::math::convert_vec(_trans.getOrigin()); }

	Ogre::Quaternion getOrientation(void) const
	{ return vl::math::convert_quat(_trans.getRotation()); }

	Transform getWorldTransform(void) const
	{
		return vl::math::convert_transform(_trans);
	}

	virtual void getWorldTransform(btTransform &worldTrans) const
	{
		worldTrans = _trans;
	}

	virtual void setWorldTransform(const btTransform &worldTrans)
	{
		_trans = worldTrans;

		if( !_visibleobj)
			return; // silently return before we set a node

		vl::Transform t = vl::math::convert_transform(_trans);
		/// @todo these should set the world transform
		_visibleobj->setOrientation(t.quaternion);
		_visibleobj->setPosition(t.position);
	}

protected:
	vl::SceneNodePtr _visibleobj;
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

#endif // VL_PHYSICS_MOTION_STATE_HPP
