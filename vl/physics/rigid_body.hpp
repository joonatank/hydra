/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-03
 *	@file rigid_body.hpp
 *
 */

#ifndef HYDRA_RIGID_BODY_HPP
#define HYDRA_RIGID_BODY_HPP

#include <bullet/BulletDynamics/Dynamics/btRigidBody.h>
#include <bullet/btBulletDynamicsCommon.h>

#include "math/conversion.hpp"

#include "motion_state.hpp"
#include "shapes.hpp"

namespace {
	using vl::math::convert_bt_vec;
	using vl::math::convert_vec;
	using vl::math::convert_bt_transform;
	using vl::math::convert_transform;
}

namespace vl {

namespace physics {

/**	@class RigidBody
 *	@brief Wrapper around Bullet rigid body
 *	Purpose is to provide Ogre compatible minimal interface
 *	Can later be used for abstracting the physics objects, so we can use
 *	dynamically loadable and user overloadable plugins.
 */
class RigidBody
{

public :
	struct ConstructionInfo
	{
		ConstructionInfo(std::string const &nam, vl::scalar m, vl::physics::MotionState *sta, 
			CollisionShapeRefPtr shap, Ogre::Vector3 const &inert)
			: name(nam), mass(m), state(sta), shape(shap), inertia(inert)
		{}

		btRigidBody::btRigidBodyConstructionInfo getBullet(void) const
		{
			btVector3 i(vl::math::convert_bt_vec(inertia));
			return btRigidBody::btRigidBodyConstructionInfo(mass, state, shape->getNative(), i);
		}

		std::string name;
		vl::scalar mass;
		vl::physics::MotionState *state;
		CollisionShapeRefPtr shape;
		Ogre::Vector3 inertia;

	};	// struct ConstructionInfo

	RigidBody(ConstructionInfo const &constructionInfo)
		: _name(constructionInfo.name), _shape(constructionInfo.shape), _bt_body(0)
	{
		_bt_body = new btRigidBody(constructionInfo.getBullet());
	}

	Ogre::Vector3 getTotalForce(void) const
	{ return convert_vec(_bt_body->getTotalForce()); }
	Ogre::Vector3 getTotalTorque(void) const
	{ return convert_vec(_bt_body->getTotalTorque()); }
	void applyForce(Ogre::Vector3 const &force, Ogre::Vector3 const &rel_pos)
	{ _bt_body->applyForce(convert_bt_vec(force), convert_bt_vec(rel_pos)); }
	void applyTorque(Ogre::Vector3 const &v)
	{ _bt_body->applyTorque(vl::math::convert_bt_vec(v)); }
	void applyTorqueImpulse(Ogre::Vector3 const &v)
	{ _bt_body->applyTorqueImpulse(vl::math::convert_bt_vec(v)); }
	void applyCentralForce(Ogre::Vector3 const &v, bool local = false)
	{
		/// @todo this is insanily inefficient way to do this
		/// I would think that all calculations are done in local space anyway
		/// so doing transforms from local to world seems like an extra overhead
		if(!local)
		{ _bt_body->applyCentralForce(vl::math::convert_bt_vec(v)); }
		else
		{ _bt_body->applyCentralForce(vl::math::convert_bt_vec(positionToLocal(v))); }
	}

	void applyCentralImpulse(Ogre::Vector3 const &v)
	{ _bt_body->applyCentralImpulse(vl::math::convert_bt_vec(v)); }
	void setLinearVelocity(Ogre::Vector3 const &v)
	{ _bt_body->setLinearVelocity(convert_bt_vec(v)); }
	void setAngularVelocity(Ogre::Vector3 const &v)
	{ _bt_body->setAngularVelocity(convert_bt_vec(v)); }

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

	/// Doesn't set mass if we have no inertia tensor use setMassProps
	/// it might work, if you set the mass first and the inertia tensor after that
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

	// Every object that is controlled by the user should have
	// DISABLE_DEACTIVATION set
	// Because activation only happens when other bodies are come near
	// NOTE might work also by using body->activate() before moving it
	void setUserControlled(bool enabled)
	{
		if(enabled)
		{ _bt_body->setActivationState(DISABLE_DEACTIVATION); }
		else
		{ _bt_body->setActivationState(ISLAND_SLEEPING); }
	}

	bool isUserControlled(void) const
	{ return _bt_body->getActivationState() == DISABLE_DEACTIVATION; }

	void setActivationState(int state)
	{ _bt_body->setActivationState(state); }
	
	MotionState *getMotionState(void)
	{ return (MotionState *)_bt_body->getMotionState(); }

	MotionState const *getMotionState(void) const
	{ return (MotionState const *)_bt_body->getMotionState(); }

	void setMotionState(MotionState *motionState)
	{ _bt_body->setMotionState(motionState); }

	vl::Transform transformToLocal(vl::Transform const &t) const
	{
		MotionState *ms = (MotionState *)_bt_body->getMotionState();
		vl::Transform from_world = ms->getWorldTransform();
		from_world.invert();
		return from_world*t;
	}

	Ogre::Vector3 positionToLocal(Ogre::Vector3 const &v) const
	{
		MotionState *ms = (MotionState *)_bt_body->getMotionState();
		vl::Transform from_world = ms->getWorldTransform();
		from_world.invert();
		return from_world*v;
	}

	vl::Transform getWorldTransform(void) const
	{ return ((MotionState *)_bt_body->getMotionState())->getWorldTransform(); }

	std::string const &getName(void) const
	{ return _name; }

	CollisionShapeRefPtr getShape(void)
	{ return _shape; }

	btRigidBody *getNative(void)
	{ return _bt_body; }

private :
	std::string _name;

	CollisionShapeRefPtr _shape;

	btRigidBody *_bt_body;

};	// class RigidBody

inline std::ostream &
operator<<(std::ostream &os, RigidBody const &body)
{
	os << "RigidBody " << body.getName() << " : motion state " << *body.getMotionState();
	if( body.isUserControlled() )
	{ os << " : user controlled"; }

	// TODO add the rest

	return os;
}

}	// namespace physics

}	// namespace vl

#endif