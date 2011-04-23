/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-01
 *
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
	}

	virtual ~MotionState()
	{}

	vl::SceneNodePtr getNode(void) const
	{ return _visibleobj; }

	void setNode(vl::SceneNodePtr node)
	{
		_visibleobj = node;
	}

	Ogre::Vector3 getPosition(void) const
	{ return vl::math::convert_vec(_trans.getOrigin()); }

	Ogre::Quaternion getOrientation(void) const
	{ return vl::math::convert_quat(_trans.getRotation()); }

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
