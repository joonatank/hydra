/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-03
 *	@file physics/rigid_body.hpp
 *
 *	This file is part of Hydra a VR game engine.
 */

#ifndef HYDRA_RIGID_BODY_HPP
#define HYDRA_RIGID_BODY_HPP

// Necessary for pointers
#include "typedefs.hpp"
// Necessary for vl::scalar
#include "math/types.hpp"
// Necessary for Transform
#include "math/transform.hpp"
// Necessary for name
#include <string>

namespace vl {

namespace physics {

/**	@class RigidBody
 *	@brief Abstract interface for Physics engine rigid body
 *	Derived classes implement this for specific physics engines.
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

		std::string name;
		vl::scalar mass;
		vl::physics::MotionState *state;
		CollisionShapeRefPtr shape;
		Ogre::Vector3 inertia;

	};	// struct ConstructionInfo

	virtual ~RigidBody(void) {}

	static RigidBodyRefPtr create(ConstructionInfo const &info);

	virtual Ogre::Vector3 getTotalForce(void) const = 0;

	virtual Ogre::Vector3 getTotalTorque(void) const = 0;
	
	void applyForce(Ogre::Vector3 const &force, Ogre::Vector3 const &rel_pos, RigidBodyRefPtr ref);

	void applyForce(Ogre::Vector3 const &force, Ogre::Vector3 const &rel_pos, bool local);

	virtual void applyForce(Ogre::Vector3 const &force, Ogre::Vector3 const &rel_pos) = 0;

	virtual void applyTorque(Ogre::Vector3 const &v) = 0;
	
	virtual void applyTorqueImpulse(Ogre::Vector3 const &v) = 0;

	void applyCentralForce(Ogre::Vector3 const &force, vl::SceneNodePtr ref);

	void applyCentralForce(Ogre::Vector3 const &force, RigidBodyRefPtr ref);

	void applyCentralForce(Ogre::Vector3 const &force, bool local)
	{ applyForce(force, Ogre::Vector3::ZERO, local); }

	void applyCentralForce(Ogre::Vector3 const &force)
	{ applyForce(force, Ogre::Vector3::ZERO); }

	virtual void applyCentralImpulse(Ogre::Vector3 const &v) = 0;
	
	virtual void setLinearVelocity(Ogre::Vector3 const &v) = 0;

	virtual Ogre::Vector3 getLinearVelocity(void) const = 0;

	virtual void setAngularVelocity(Ogre::Vector3 const &v) = 0;
	
	virtual Ogre::Vector3 getAngularVelocity(void) const = 0;

	virtual void setDamping(Ogre::Real linear, Ogre::Real angular) = 0;

	virtual Ogre::Real getInvMass(void) = 0;

	virtual void clearForces(void) = 0;

	void setLinearDamping(Ogre::Real damp)
	{ setDamping(damp, getAngularDamping()); }

	virtual Ogre::Real getLinearDamping(void) const = 0;

	void setAngularDamping(Ogre::Real damp)
	{ setDamping(getLinearDamping(), damp); }

	virtual Ogre::Real getAngularDamping(void) const = 0;

	virtual void translate(Ogre::Vector3 const &v) = 0;

	/// Doesn't set mass if we have no inertia tensor use setMassProps
	/// it might work, if you set the mass first and the inertia tensor after that
 	virtual void setMass(Ogre::Real mass) = 0;

	virtual Ogre::Real getMass(void) const = 0;

	virtual void setInertia(Ogre::Vector3 const &inertia) = 0;

	virtual void setMassProps(Ogre::Real mass, Ogre::Vector3 const &inertia) = 0;

	virtual void setCenterOfMassTransform(vl::Transform const &xform) = 0;

	virtual vl::Transform getCenterOfMassTransform(void) const = 0;

	// Every object that is controlled by the user should have
	// DISABLE_DEACTIVATION set
	// Because activation only happens when other bodies are come near
	// NOTE might work also by using body->activate() before moving it
	virtual void setUserControlled(bool enabled) = 0;

	virtual bool isUserControlled(void) const = 0;

	virtual void setActivationState(int state) = 0;
	
	virtual MotionState *getMotionState(void) = 0;

	virtual MotionState const *getMotionState(void) const = 0;

	virtual void setMotionState(MotionState *motionState) = 0;

	vl::Transform transformToLocal(vl::Transform const &t) const;

	Ogre::Vector3 positionToLocal(Ogre::Vector3 const &v) const;

	virtual vl::Transform getWorldTransform(void) const = 0;

	virtual void setWorldTransform(Transform const &worldTrans) = 0;

	std::string const &getName(void) const
	{ return _name; }

	CollisionShapeRefPtr getShape(void)
	{ return _shape; }

protected :
	RigidBody(ConstructionInfo const &info);

	std::string _name;

	CollisionShapeRefPtr _shape;

};	// class RigidBody

std::ostream &operator<<(std::ostream &os, RigidBody const &body);

}	// namespace physics

}	// namespace vl

#endif
