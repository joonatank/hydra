/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-05
 *	@file physics/physics_constraints.hpp
 *
 *	This file is part of Hydra a VR game engine.
 */

#ifndef HYDRA_PHYSICS_CONSTRAINTS_HPP
#define HYDRA_PHYSICS_CONSTRAINTS_HPP

#include "math/transform.hpp"

#include "rigid_body.hpp"

namespace vl
{

namespace physics
{

/// The base constraint for physics needs to be abstract interface
class Constraint
{
public :
	virtual RigidBodyRefPtr getBodyA(void) = 0;

	virtual RigidBodyRefPtr getBodyB(void) = 0;

	virtual ~Constraint(void) {}
};

/// @class SixDofConstraint
/// Generic six dof constraint with a spring damper system
class SixDofConstraint : public vl::physics::Constraint
{
public :
	virtual ~SixDofConstraint(void) {}

	RigidBodyRefPtr getBodyA(void)
	{ return _bodyA.lock(); }

	RigidBodyRefPtr getBodyB(void)
	{ return _bodyB.lock(); }

	virtual void setLinearLowerLimit(Ogre::Vector3 const &linearLower) = 0;
	
	virtual void setLinearUpperLimit(Ogre::Vector3 const &linearUpper) = 0;

	virtual void setAngularLowerLimit(Ogre::Vector3 const &angularLower) = 0;

	virtual void setAngularUpperLimit(Ogre::Vector3 const &angularUpper) = 0;

	/// Index 0-2 for translation (x, y, z)
	/// Index 3-5 for rotations (x, y, z)
	virtual void enableSpring(int index, bool onOff) = 0;
	virtual void setStiffness(int index, vl::scalar stiffness) = 0;
	virtual void setDamping(int index, vl::scalar damping) = 0;
	virtual void setEquilibriumPoint(void) = 0;
	virtual void setEquilibriumPoint(int index) = 0;
	
	// @todo add motors

	static SixDofConstraintRefPtr create(RigidBodyRefPtr rbA, RigidBodyRefPtr rbB, 
		Transform const &frameInA, Transform const &frameInB, bool useLinearReferenceFrameA);

protected :
	SixDofConstraint(RigidBodyRefPtr rbA, RigidBodyRefPtr rbB, 
		Transform const &frameInA, Transform const &frameInB, bool useLinearReferenceFrameA)
		: _bodyA(rbA)
		, _bodyB(rbB)
	{}


	RigidBodyWeakPtr _bodyA;
	RigidBodyWeakPtr _bodyB;

};	// class SixDofConstraint

class SliderConstraint : public vl::physics::Constraint
{
public :
	virtual ~SliderConstraint(void) {}

	RigidBodyRefPtr getBodyA(void)
	{ return _bodyA.lock(); }

	RigidBodyRefPtr getBodyB(void)
	{ return _bodyB.lock(); }

	virtual vl::scalar getLowerLinLimit(void) const = 0;
	
	virtual void setLowerLinLimit(vl::scalar lowerLimit) = 0;

	virtual vl::scalar getUpperLinLimit(void) const = 0;
	
	virtual void setUpperLinLimit(vl::scalar upperLimit) = 0;
	
	virtual vl::scalar getLowerAngLimit(void) = 0;

	virtual void setLowerAngLimit(vl::scalar lowerLimit) = 0;	

	virtual vl::scalar getUpperAngLimit(void) = 0;

	virtual void setUpperAngLimit(vl::scalar upperLimit) = 0;
	
	virtual bool getUseLinearReferenceFrameA(void) = 0;
	
	virtual vl::scalar getSoftnessDirLin(void) = 0;
	
	virtual vl::scalar getRestitutionDirLin(void) = 0;

	virtual vl::scalar getDampingDirLin(void) = 0;
	
	virtual vl::scalar getSoftnessDirAng(void) = 0;
	
	virtual vl::scalar getRestitutionDirAng(void) = 0;
	
	virtual vl::scalar getDampingDirAng(void) = 0;
	
	virtual vl::scalar getSoftnessLimLin(void) = 0;
	
	virtual vl::scalar getRestitutionLimLin(void) = 0;
	virtual vl::scalar getDampingLimLin(void) = 0;
	
	virtual vl::scalar getSoftnessLimAng(void) = 0;
	
	virtual vl::scalar getRestitutionLimAng(void) = 0;
	
	virtual vl::scalar getDampingLimAng(void) = 0;
	
	virtual vl::scalar getSoftnessOrthoLin(void) = 0;
	
	virtual vl::scalar getRestitutionOrthoLin(void) = 0;
	
	virtual vl::scalar getDampingOrthoLin(void) = 0;
	
	virtual vl::scalar getSoftnessOrthoAng(void) = 0;

	virtual vl::scalar getRestitutionOrthoAng(void) = 0;

	virtual vl::scalar getDampingOrthoAng(void) = 0;

	virtual void setSoftnessDirLin(vl::scalar softnessDirLin) = 0;
	
	virtual void setRestitutionDirLin(vl::scalar restitutionDirLin) = 0;
	
	virtual void setDampingDirLin(vl::scalar dampingDirLin) = 0;
	
	virtual void setSoftnessDirAng(vl::scalar softnessDirAng) = 0;
	
	virtual void setRestitutionDirAng(vl::scalar restitutionDirAng) = 0;
	
	virtual void setDampingDirAng(vl::scalar dampingDirAng) = 0;
	
	virtual void setSoftnessLimLin(vl::scalar softnessLimLin) = 0;

	virtual void setRestitutionLimLin(vl::scalar restitutionLimLin) = 0;
	
	virtual void setDampingLimLin(vl::scalar dampingLimLin) = 0;
	
	virtual void setSoftnessLimAng(vl::scalar softnessLimAng) = 0;
	
	virtual void setRestitutionLimAng(vl::scalar restitutionLimAng) = 0;

	virtual void setDampingLimAng(vl::scalar dampingLimAng) = 0;
	
	virtual void setSoftnessOrthoLin(vl::scalar softnessOrthoLin) = 0;
	
	virtual void setRestitutionOrthoLin(vl::scalar restitutionOrthoLin) = 0;
	
	virtual void setDampingOrthoLin(vl::scalar dampingOrthoLin) = 0;

	virtual void setSoftnessOrthoAng(vl::scalar softnessOrthoAng) = 0;
	
	virtual void setRestitutionOrthoAng(vl::scalar restitutionOrthoAng) = 0;
	
	virtual void setDampingOrthoAng(vl::scalar dampingOrthoAng) = 0;


	// Motor
	virtual void setPoweredLinMotor(bool onOff) = 0;
	
	virtual bool getPoweredLinMotor(void) = 0;

	void addTargetLinMotorVelocity(vl::scalar velocity)
	{ setTargetLinMotorVelocity(velocity + getTargetLinMotorVelocity()); }

	virtual void setTargetLinMotorVelocity(vl::scalar targetLinMotorVelocity) = 0;
	
	virtual vl::scalar getTargetLinMotorVelocity(void) = 0;

	virtual void setMaxLinMotorForce(vl::scalar maxLinMotorForce) = 0;

	virtual vl::scalar getMaxLinMotorForce(void) = 0;

	virtual void setPoweredAngMotor(bool onOff) = 0;
	
	virtual bool getPoweredAngMotor(void) = 0;

	virtual void setTargetAngMotorVelocity(vl::scalar targetAngMotorVelocity) = 0;

	virtual vl::scalar getTargetAngMotorVelocity(void) = 0;
	
	virtual void setMaxAngMotorForce(vl::scalar maxAngMotorForce) = 0;
	
	virtual vl::scalar getMaxAngMotorForce(void) = 0;


	static SliderConstraintRefPtr create(RigidBodyRefPtr rbA, RigidBodyRefPtr rbB, 
		Transform const &frameInA, Transform const &frameInB, bool useLinearReferenceFrameA);

protected :
	SliderConstraint(RigidBodyRefPtr rbA, RigidBodyRefPtr rbB, 
		Transform const &frameInA, Transform const &frameInB, bool useLinearReferenceFrameA)
		: _bodyA(rbA)
		, _bodyB(rbB)
	{}

	RigidBodyWeakPtr _bodyA;
	RigidBodyWeakPtr _bodyB;

};	// class SliderConstraint

class HingeConstraint : public vl::physics::Constraint
{
public :
	virtual ~HingeConstraint(void) {}

	RigidBodyRefPtr getBodyA(void)
	{ return _bodyA.lock(); }

	RigidBodyRefPtr getBodyB(void)
	{ return _bodyB.lock(); }

	virtual void setAngularOnly(bool angularOnly) = 0;

	virtual void enableAngularMotor(bool enableMotor, vl::scalar targetVelocity, vl::scalar maxMotorImpulse) = 0;

	virtual void enableMotor(bool enableMotor) = 0;
	
	virtual void setMaxMotorImpulse(vl::scalar maxMotorImpulse) = 0;

	virtual void setMotorTarget(vl::scalar targetAngle, vl::scalar dt) = 0;

	virtual void setLimit(Ogre::Radian const &low, Ogre::Radian const &high, vl::scalar softness=0.9f, vl::scalar biasFactor=0.3f, vl::scalar relaxationFactor=1.0f) = 0;

	virtual void setAxis(Ogre::Vector3 &axisInA) = 0;
	
	virtual vl::scalar getLowerLimit(void) const = 0;

	virtual vl::scalar getUpperLimit(void) const = 0;

	virtual vl::scalar getHingeAngle(void) = 0;
	
	static HingeConstraintRefPtr create(RigidBodyRefPtr rbA, RigidBodyRefPtr rbB, 
		Transform const &frameInA, Transform const &frameInB, bool useLinearReferenceFrameA);

protected :
	HingeConstraint(RigidBodyRefPtr rbA, RigidBodyRefPtr rbB, 
		Transform const &frameInA, Transform const &frameInB, bool useLinearReferenceFrameA)
		: _bodyA(rbA)
		, _bodyB(rbB)
	{}

	RigidBodyWeakPtr _bodyA;
	RigidBodyWeakPtr _bodyB;

};	// class HingeConstraint

}	// namespace physics

}	// namepsace vl

#endif	// HYDRA_PHYSICS_CONSTRAINTS_HPP
