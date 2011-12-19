/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-08
 *	@file physics/rigid_body_bullet.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef HYDRA_RIGID_BODY_BULLET_HPP
#define HYDRA_RIGID_BODY_BULLET_HPP

/// Base class
#include "rigid_body.hpp"

/// Necessary for converting vectors and quaternions
#include "math/conversion.hpp"
/// Necessary for casting collision shapes
#include "shapes_bullet.hpp"
/// Necessary for passing MotionState
#include "motion_state.hpp"

/// Engine implementation
#include <bullet/BulletDynamics/Dynamics/btRigidBody.h>
#include <bullet/btBulletDynamicsCommon.h>


namespace 
{
	using vl::math::convert_bt_vec;
	using vl::math::convert_vec;
	using vl::math::convert_bt_transform;
	using vl::math::convert_transform;

	btRigidBody::btRigidBodyConstructionInfo convert_construction_info(vl::physics::RigidBody::ConstructionInfo const &info)
	{
		using vl::physics::BulletCollisionShape;
		using vl::physics::BulletCollisionShapeRefPtr;

		btVector3 inertia(convert_bt_vec(info.inertia));
		BulletCollisionShapeRefPtr shape = boost::dynamic_pointer_cast<BulletCollisionShape>(info.shape);
		if(info.mass == 0)
		{ inertia = btVector3(0, 0, 0); }

		return btRigidBody::btRigidBodyConstructionInfo(info.mass, info.state, shape->getNative(), inertia);
	}
}

namespace vl
{

namespace physics
{

/**	@class BulletRigidBody
 *	@brief Wrapper around Bullet rigid body
 *	Purpose is to provide Ogre compatible minimal interface
 *	Can later be used for abstracting the physics objects, so we can use
 *	dynamically loadable and user overloadable plugins.
 */

class BulletRigidBody : public vl::physics::RigidBody
{
public :
	BulletRigidBody(RigidBody::ConstructionInfo const &info)
		: RigidBody(info)
		, _bt_body(0)
	{
		_bt_body = new btRigidBody(convert_construction_info(info));
	}

	// @todo can we delete the body here?
	virtual ~BulletRigidBody(void)
	{
		delete _bt_body;
	}

	virtual Ogre::Vector3 getTotalForce(void) const
	{ return convert_vec(_bt_body->getTotalForce()); }
	virtual Ogre::Vector3 getTotalTorque(void) const
	{ return convert_vec(_bt_body->getTotalTorque()); }

	void applyForce(Ogre::Vector3 const &force, Ogre::Vector3 const &rel_pos)
	{ _bt_body->applyForce(convert_bt_vec(force), convert_bt_vec(rel_pos)); }

	void applyTorque(Ogre::Vector3 const &v)
	{ _bt_body->applyTorque(vl::math::convert_bt_vec(v)); }

	void applyTorqueImpulse(Ogre::Vector3 const &v)
	{ _bt_body->applyTorqueImpulse(vl::math::convert_bt_vec(v)); }

	void applyCentralImpulse(Ogre::Vector3 const &v)
	{ _bt_body->applyCentralImpulse(vl::math::convert_bt_vec(v)); }

	void setLinearVelocity(Ogre::Vector3 const &v)
	{ _bt_body->setLinearVelocity(convert_bt_vec(v)); }

	Ogre::Vector3 getLinearVelocity(void) const
	{ return convert_vec(_bt_body->getLinearVelocity()); }

	void setAngularVelocity(Ogre::Vector3 const &v)
	{ _bt_body->setAngularVelocity(convert_bt_vec(v)); }

	Ogre::Vector3 getAngularVelocity(void) const
	{ return convert_vec(_bt_body->getAngularVelocity()); }

	void setDamping(Ogre::Real linear, Ogre::Real angular)
	{ _bt_body->setDamping(linear, angular); }

	Ogre::Real getInvMass(void)
	{ return _bt_body->getInvMass(); }

	void clearForces(void)
	{ _bt_body->clearForces(); }

	Ogre::Real getLinearDamping(void) const
	{ return _bt_body->getLinearDamping(); }

	Ogre::Real getAngularDamping(void) const
	{ return _bt_body->getAngularDamping(); }

	void translate(Ogre::Vector3 const &v)
	{ _bt_body->translate(convert_bt_vec(v)); }

	void setMass(Ogre::Real mass)
	{
		if(!_bt_body->getInvInertiaDiagLocal().isZero())
		{
			btVector3 inv = _bt_body->getInvInertiaDiagLocal();
			btVector3 inertia(1/inv.x(), 1/inv.y(), 1/inv.z());
			_bt_body->setMassProps(mass, inertia);
		}
	}

	Ogre::Real getMass(void) const
	{
		if(_bt_body->getInvMass() != 0)
		{ return 1/_bt_body->getInvMass(); }
		else
		{ return 0; }
	}

	void setInertia(Ogre::Vector3 const &inertia)
	{ setMassProps( 1/getInvMass(), inertia ); }

	void setMassProps(Ogre::Real mass, Ogre::Vector3 const &inertia)
	{ _bt_body->setMassProps(mass, convert_bt_vec(inertia)); }

	void setCenterOfMassTransform(vl::Transform const &xform)
	{ _bt_body->setCenterOfMassTransform(convert_bt_transform(xform)); }

	vl::Transform getCenterOfMassTransform(void) const
	{ return convert_transform(_bt_body->getCenterOfMassTransform()); }

	virtual void setUserControlled(bool enabled)
	{
		if(enabled)
		{ _bt_body->setActivationState(DISABLE_DEACTIVATION); }
		else
		{ _bt_body->setActivationState(ISLAND_SLEEPING); }
	}

	virtual bool isUserControlled(void) const
	{ return _bt_body->getActivationState() == DISABLE_DEACTIVATION; }

	virtual void setActivationState(int state)
	{ _bt_body->setActivationState(state); }

	virtual MotionState *getMotionState(void)
	{ return (MotionState *)_bt_body->getMotionState(); }

	virtual MotionState const *getMotionState(void) const
	{ return (MotionState const *)_bt_body->getMotionState(); }

	virtual void setMotionState(MotionState *motionState)
	{ _bt_body->setMotionState(motionState); }

	virtual vl::Transform getWorldTransform(void) const
	{ return convert_transform(_bt_body->getWorldTransform()); }

	virtual void setWorldTransform(Transform const &worldTrans)
	{ _bt_body->setWorldTransform(convert_bt_transform(worldTrans)); }

	virtual void enableKinematicObject(bool enable)
	{
		if(enable)
		{
			_bt_body->setCollisionFlags(_bt_body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
		}
		else
		{
			_bt_body->setCollisionFlags(_bt_body->getCollisionFlags() && ~btCollisionObject::CF_KINEMATIC_OBJECT);
		}
	}

	virtual void disableCollisions(void)
	{
		_bt_body->setCollisionFlags(_bt_body->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
	}

	virtual void enableCollisions(void)
	{
		_bt_body->setCollisionFlags(_bt_body->getCollisionFlags() && ~btCollisionObject::CF_NO_CONTACT_RESPONSE);
	}

	virtual bool isCollisionsDisabled(void) const
	{
		return _bt_body->getCollisionFlags() && btCollisionObject::CF_NO_CONTACT_RESPONSE;
	}


	virtual bool isKinematicObject(void) const
	{ return _bt_body->getCollisionFlags() && btCollisionObject::CF_KINEMATIC_OBJECT; }

	btRigidBody *getNative(void)
	{ return _bt_body; }

private :
	btRigidBody *_bt_body;

};	// class BulletRigidBody

typedef boost::shared_ptr<BulletRigidBody> BulletRigidBodyRefPtr;

}	// namespace physics

}	// namespace vl

#endif	// HYDRA_RIGID_BODY_BULLET_HPP
