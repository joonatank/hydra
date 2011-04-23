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

namespace {
	using vl::math::convert_bt_vec;
	using vl::math::convert_vec;
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
	RigidBody(std::string const &name, btRigidBody::btRigidBodyConstructionInfo const &constructionInfo)
		: _name(name), _bt_body(0)
	{
		_bt_body = new btRigidBody(constructionInfo);
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
	void applyCentralForce(Ogre::Vector3 const &v)
	{ _bt_body->applyCentralForce(vl::math::convert_bt_vec(v)); }
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

// 	void setMass(Ogre::Real mass);
// 	{ return _bt_body-> }

	void setInertia(Ogre::Vector3 const &inertia)
	{ setMassProps( 1/getInvMass(), inertia ); }

	void setMassProps(Ogre::Real mass, Ogre::Vector3 const &inertia)
	{ _bt_body->setMassProps(mass, convert_bt_vec(inertia)); }

	// Every object that is controlled by the user should have
	// DISABLE_DEACTIVATION set
	// Because activation only happens when other bodies are come near
	// NOTE might work also by using body->activate() before moving it
	void setUserControlled(void)
	{ _bt_body->setActivationState(DISABLE_DEACTIVATION); }

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

	std::string const &getName(void) const
	{ return _name; }

	btRigidBody *getNative(void)
	{ return _bt_body; }

private :
	std::string _name;

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