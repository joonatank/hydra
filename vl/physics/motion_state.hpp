/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2011-01
 *
 */

#ifndef VL_PHYSICS_MOTION_STATE_HPP
#define VL_PHYSICS_MOTION_STATE_HPP

// This class initialises Bullet physics so they are necessary
#include <bullet/btBulletDynamicsCommon.h>
#include <math/math.hpp>

namespace vl
{

namespace physics
{

/**
 *
 */
// TODO move to own header
class MotionState : public btMotionState
{
public:
	MotionState( Ogre::Vector3 const &pos,
				 Ogre::Quaternion const &orient,
				 eqOgre::SceneNode *node = 0)
		: _visibleobj(node)
	{
		btQuaternion q(orient.x, orient.y, orient.z, orient.w);
		btVector3 v(pos.x, pos.y, pos.z);
		_trans = btTransform( q, v );
	}

	MotionState( vl::Transform const &trans,
				 eqOgre::SceneNode *node = 0)
		: _visibleobj(node)
	{
		Ogre::Vector3 const &pos = trans.position;
		Ogre::Quaternion const &orient = trans.quaternion;
		btQuaternion q(orient.x, orient.y, orient.z, orient.w);
		btVector3 v(pos.x, pos.y, pos.z);
		_trans = btTransform( q, v );
	}

	virtual ~MotionState()
	{}

	void setNode(eqOgre::SceneNode *node)
	{
		_visibleobj = node;
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

		btQuaternion rot = worldTrans.getRotation();
		Ogre::Quaternion ogre_rot( rot.w(), rot.x(), rot.y(), rot.z() );
		_visibleobj->setOrientation( ogre_rot );
		btVector3 pos = worldTrans.getOrigin();
		Ogre::Vector3 ogre_vec( pos.x(), pos.y(), pos.z() );
		_visibleobj->setPosition( ogre_vec );
	}

protected:
	eqOgre::SceneNode *_visibleobj;
	btTransform _trans;

};	// class MotionState

}	// namespace physics

}	// namespace vl

#endif // VL_PHYSICS_MOTION_STATE_HPP