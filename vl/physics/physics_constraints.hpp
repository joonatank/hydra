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

#include <bullet/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h>

#include "constraints.hpp"

namespace {
	using vl::math::convert_bt_vec;
	using vl::math::convert_vec;
	using vl::math::convert_bt_transform;
}

namespace vl
{

namespace physics
{

class Constraint : public vl::Constraint
{
public :
	virtual btTypedConstraint *getNative(void) = 0;

private :
};

class SixDofConstraint : public Constraint
{
public :
	void setLinearLowerLimit(Ogre::Vector3 const &linearLower)
	{ _bt_constraint->setLinearLowerLimit(convert_bt_vec(linearLower)); }
	
	void setLinearUpperLimit(Ogre::Vector3 const &linearUpper)
	{ _bt_constraint->setLinearUpperLimit(convert_bt_vec(linearUpper)); }

	void setAngularLowerLimit(Ogre::Vector3 const &angularLower)
	{ _bt_constraint->setAngularLowerLimit(convert_bt_vec(angularLower)); }

	void setAngularUpperLimit(Ogre::Vector3 const &angularUpper)
	{ _bt_constraint->setAngularUpperLimit(convert_bt_vec(angularUpper)); }

	// @todo add motors

	RigidBodyRefPtr getBodyA(void)
	{ return _bodyA.lock(); }

	RigidBodyRefPtr getBodyB(void)
	{ return _bodyB.lock(); }

	virtual btTypedConstraint *getNative(void)
	{ return _bt_constraint; }

	static SixDofConstraintRefPtr create(RigidBodyRefPtr rbA, RigidBodyRefPtr rbB, 
		Transform const &frameInA, Transform const &frameInB, bool useLinearReferenceFrameA)
	{
		SixDofConstraintRefPtr constraint(new SixDofConstraint(rbA, rbB, frameInA, frameInB, useLinearReferenceFrameA));
		return constraint;
	}

private :
	SixDofConstraint(RigidBodyRefPtr rbA, RigidBodyRefPtr rbB, 
		Transform const &frameInA, Transform const &frameInB, bool useLinearReferenceFrameA)
		: _bt_constraint(0)
		, _bodyA(rbA)
		, _bodyB(rbB)
	{
		_bt_constraint = new btGeneric6DofConstraint(*rbA->getNative(), 
			*rbB->getNative(), convert_bt_transform(frameInA), 
			convert_bt_transform(frameInB), useLinearReferenceFrameA);
	}

 	SixDofConstraint(RigidBodyRefPtr rbB, Transform const &frameInB, bool useLinearReferenceFrameB)
		: _bt_constraint(0)
		, _bodyB(rbB)
	{
		_bt_constraint = new btGeneric6DofConstraint(*rbB->getNative(), 
			convert_bt_transform(frameInB), useLinearReferenceFrameB);
	}

	btGeneric6DofConstraint *_bt_constraint;

	RigidBodyWeakPtr _bodyA;
	RigidBodyWeakPtr _bodyB;

};	// class SixDofConstraint

class SliderConstraint : public Constraint
{
public :
	RigidBodyRefPtr getBodyA(void)
	{ return _bodyA.lock(); }

	RigidBodyRefPtr getBodyB(void)
	{ return _bodyB.lock(); }

	vl::scalar getLowerLinLimit(void) const
	{ return _bt_constraint->getLowerLinLimit(); }
	void setLowerLinLimit(vl::scalar lowerLimit)
	{ _bt_constraint->setLowerLinLimit(lowerLimit); }

	vl::scalar getUpperLinLimit(void) const
	{ return _bt_constraint->getUpperLinLimit(); }
	void setUpperLinLimit(vl::scalar upperLimit)
	{ _bt_constraint->setUpperLinLimit(upperLimit); }

	vl::scalar getLowerAngLimit(void)
	{ return _bt_constraint->getLowerAngLimit(); }
	void setLowerAngLimit(vl::scalar lowerLimit)
	{ _bt_constraint->setLowerAngLimit(lowerLimit); }

	vl::scalar getUpperAngLimit(void)
	{ return _bt_constraint->getUpperAngLimit(); }
	void setUpperAngLimit(vl::scalar upperLimit)
	{ _bt_constraint->setUpperAngLimit(upperLimit); }

	bool getUseLinearReferenceFrameA(void)
	{ return _bt_constraint->getUseLinearReferenceFrameA(); }
	vl::scalar getSoftnessDirLin(void)
	{ return _bt_constraint->getSoftnessDirLin(); }
	vl::scalar getRestitutionDirLin(void)
	{ return _bt_constraint->getRestitutionDirLin(); }
	vl::scalar getDampingDirLin(void)
	{ return _bt_constraint->getDampingDirLin(); }
	vl::scalar getSoftnessDirAng(void)
	{ return _bt_constraint->getSoftnessDirAng(); }
	vl::scalar getRestitutionDirAng(void)
	{ return _bt_constraint->getRestitutionDirAng(); }
	vl::scalar getDampingDirAng(void)
	{ return _bt_constraint->getDampingDirAng(); }
	vl::scalar getSoftnessLimLin(void)
	{ return _bt_constraint->getSoftnessLimLin(); }
	vl::scalar getRestitutionLimLin(void)
	{ return _bt_constraint->getRestitutionLimLin(); }
	vl::scalar getDampingLimLin(void)
	{ return _bt_constraint->getDampingLimLin(); }
	vl::scalar getSoftnessLimAng(void)
	{ return _bt_constraint->getSoftnessLimAng(); }
	vl::scalar getRestitutionLimAng(void)
	{ return _bt_constraint->getRestitutionLimAng(); }
	vl::scalar getDampingLimAng(void)
	{ return _bt_constraint->getDampingLimAng(); }
	vl::scalar getSoftnessOrthoLin(void)
	{ return _bt_constraint->getSoftnessOrthoLin(); }
	vl::scalar getRestitutionOrthoLin(void)
	{ return _bt_constraint->getRestitutionOrthoLin(); }
	vl::scalar getDampingOrthoLin(void)
	{ return _bt_constraint->getDampingOrthoLin(); }
	vl::scalar getSoftnessOrthoAng(void)
	{ return _bt_constraint->getSoftnessOrthoAng(); }
	vl::scalar getRestitutionOrthoAng(void)
	{ return _bt_constraint->getRestitutionOrthoAng(); }
	vl::scalar getDampingOrthoAng(void)
	{ return _bt_constraint->getDampingOrthoAng(); }

	void setSoftnessDirLin(vl::scalar softnessDirLin)
	{ _bt_constraint->setSoftnessDirLin(softnessDirLin); }
	void setRestitutionDirLin(vl::scalar restitutionDirLin)
	{ _bt_constraint->setRestitutionDirLin(restitutionDirLin); }
	void setDampingDirLin(vl::scalar dampingDirLin)
	{ _bt_constraint->setDampingDirLin(dampingDirLin); }
	void setSoftnessDirAng(vl::scalar softnessDirAng)
	{ _bt_constraint->setSoftnessDirAng(softnessDirAng); }
	void setRestitutionDirAng(vl::scalar restitutionDirAng)
	{ _bt_constraint->setRestitutionDirAng(restitutionDirAng); }
	void setDampingDirAng(vl::scalar dampingDirAng)
	{ _bt_constraint->setDampingDirAng(dampingDirAng); }
	void setSoftnessLimLin(vl::scalar softnessLimLin)
	{ _bt_constraint->setSoftnessLimLin(softnessLimLin); }
	void setRestitutionLimLin(vl::scalar restitutionLimLin)
	{ _bt_constraint->setRestitutionLimLin(restitutionLimLin); }
	void setDampingLimLin(vl::scalar dampingLimLin)
	{ _bt_constraint->setDampingLimLin(dampingLimLin); }
	void setSoftnessLimAng(vl::scalar softnessLimAng)
	{ _bt_constraint->setSoftnessLimAng(softnessLimAng); }
	void setRestitutionLimAng(vl::scalar restitutionLimAng)
	{ _bt_constraint->setRestitutionLimAng(restitutionLimAng); }
	void setDampingLimAng(vl::scalar dampingLimAng)
	{ _bt_constraint->setDampingLimAng(dampingLimAng); }
	void setSoftnessOrthoLin(vl::scalar softnessOrthoLin)
	{ _bt_constraint->setSoftnessOrthoLin(softnessOrthoLin); }
	void setRestitutionOrthoLin(vl::scalar restitutionOrthoLin)
	{ _bt_constraint->setRestitutionOrthoLin(restitutionOrthoLin); }
	void setDampingOrthoLin(vl::scalar dampingOrthoLin)
	{ _bt_constraint->setDampingOrthoLin(dampingOrthoLin); }
	void setSoftnessOrthoAng(vl::scalar softnessOrthoAng)
	{ _bt_constraint->setSoftnessOrthoAng(softnessOrthoAng); }
	void setRestitutionOrthoAng(vl::scalar restitutionOrthoAng)
	{ _bt_constraint->setRestitutionOrthoAng(restitutionOrthoAng); }
	void setDampingOrthoAng(vl::scalar dampingOrthoAng)
	{ _bt_constraint->setDampingOrthoAng(dampingOrthoAng); }

	// Motor
	void setPoweredLinMotor(bool onOff)
	{ _bt_constraint->setPoweredLinMotor(onOff); }
	bool getPoweredLinMotor(void)
	{ return _bt_constraint->getPoweredLinMotor(); }

	void addTargetLinMotorVelocity(vl::scalar velocity)
	{ setTargetLinMotorVelocity(velocity + getTargetLinMotorVelocity()); }

	void setTargetLinMotorVelocity(vl::scalar targetLinMotorVelocity)
	{ _bt_constraint->setTargetLinMotorVelocity(targetLinMotorVelocity); }
	vl::scalar getTargetLinMotorVelocity(void)
	{ return _bt_constraint->getTargetLinMotorVelocity(); }

	void setMaxLinMotorForce(vl::scalar maxLinMotorForce)
	{ _bt_constraint->setMaxLinMotorForce(maxLinMotorForce); }
	vl::scalar getMaxLinMotorForce(void)
	{ return _bt_constraint->getMaxLinMotorForce(); }
	
	void setPoweredAngMotor(bool onOff)
	{ _bt_constraint->setPoweredAngMotor(onOff); }
	bool getPoweredAngMotor(void)
	{ return _bt_constraint->getPoweredAngMotor(); }

	void setTargetAngMotorVelocity(vl::scalar targetAngMotorVelocity)
	{ _bt_constraint->setTargetAngMotorVelocity(targetAngMotorVelocity); }
	vl::scalar getTargetAngMotorVelocity(void)
	{ return _bt_constraint->getTargetAngMotorVelocity(); }

	void setMaxAngMotorForce(vl::scalar maxAngMotorForce)
	{ _bt_constraint->setMaxAngMotorForce(maxAngMotorForce); }
	vl::scalar getMaxAngMotorForce(void)
	{ return _bt_constraint->getMaxAngMotorForce(); }

	virtual btTypedConstraint *getNative(void)
	{ return _bt_constraint; }

	static SliderConstraintRefPtr create(RigidBodyRefPtr rbA, RigidBodyRefPtr rbB, 
		Transform const &frameInA, Transform const &frameInB, bool useLinearReferenceFrameA)
	{
		SliderConstraintRefPtr constraint(new SliderConstraint(rbA, rbB, frameInA, frameInB, useLinearReferenceFrameA));
		return constraint;
	}

private :
	SliderConstraint(RigidBodyRefPtr rbA, RigidBodyRefPtr rbB, 
		Transform const &frameInA, Transform const &frameInB, bool useLinearReferenceFrameA)
		: _bt_constraint(0)
		, _bodyA(rbA)
		, _bodyB(rbB)
	{
		_bt_constraint = new btSliderConstraint(*rbA->getNative(), *rbB->getNative(), 
			convert_bt_transform(frameInA), convert_bt_transform(frameInB), useLinearReferenceFrameA);
	}

 	SliderConstraint(RigidBodyRefPtr rbB, Transform const &frameInB, bool useLinearReferenceFrameB)
		: _bt_constraint(0)
		, _bodyB(rbB)
	{
		_bt_constraint = new btSliderConstraint(*rbB->getNative(), 
			convert_bt_transform(frameInB), useLinearReferenceFrameB);
	}

	btSliderConstraint *_bt_constraint;

	RigidBodyWeakPtr _bodyA;
	RigidBodyWeakPtr _bodyB;

};	// class SliderConstraint

class HingeConstraint : public Constraint
{
public :
	virtual btTypedConstraint *getNative(void)
	{ return _bt_constraint; }

	static HingeConstraintRefPtr create(RigidBodyRefPtr rbA, RigidBodyRefPtr rbB, 
		Transform const &frameInA, Transform const &frameInB, bool useLinearReferenceFrameA)
	{
		HingeConstraintRefPtr constraint(new HingeConstraint(rbA, rbB, frameInA, frameInB, useLinearReferenceFrameA));
		return constraint;
	}

	void setAngularOnly(bool angularOnly)
	{ _bt_constraint->setAngularOnly(angularOnly); }

	void enableAngularMotor(bool enableMotor, vl::scalar targetVelocity, vl::scalar maxMotorImpulse)
	{ _bt_constraint->enableAngularMotor(enableMotor, targetVelocity, maxMotorImpulse); }

	void enableMotor(bool enableMotor)
	{ _bt_constraint->enableMotor(enableMotor); }
	
	void setMaxMotorImpulse(vl::scalar maxMotorImpulse)
	{ _bt_constraint->setMaxMotorImpulse(maxMotorImpulse); }

//	void setMotorTarget(const btQuaternion &qAinB, vl::scalar dt)
//	{ return _bt_constraint->setMotorTarget(qAinB, dt); }

	void setMotorTarget(vl::scalar targetAngle, vl::scalar dt)
	{ _bt_constraint->setMotorTarget(targetAngle, dt); }

	void setLimit(vl::scalar low, vl::scalar high, vl::scalar softness=0.9f, vl::scalar biasFactor=0.3f, vl::scalar relaxationFactor=1.0f)
	{ _bt_constraint->setLimit(low, high, softness, biasFactor, relaxationFactor); }
	
	void setAxis(Ogre::Vector3 &axisInA)
	{ _bt_constraint->setAxis(vl::math::convert_bt_vec(axisInA)); }
	
	vl::scalar getLowerLimit(void) const
	{ return _bt_constraint->getLowerLimit(); }

	vl::scalar getUpperLimit(void) const
	{ return _bt_constraint->getUpperLimit(); }

	vl::scalar getHingeAngle(void)
	{ return _bt_constraint->getHingeAngle(); }

private :
	HingeConstraint(RigidBodyRefPtr rbA, RigidBodyRefPtr rbB, 
		Transform const &frameInA, Transform const &frameInB, bool useLinearReferenceFrameA)
		: _bt_constraint(0)
		, _bodyA(rbA)
		, _bodyB(rbB)
	{
		_bt_constraint = new btHingeConstraint(*rbA->getNative(), *rbB->getNative(), 
			convert_bt_transform(frameInA), convert_bt_transform(frameInB), useLinearReferenceFrameA);
	}

 	HingeConstraint(RigidBodyRefPtr rbB, Transform const &frameInB, bool useLinearReferenceFrameB)
		: _bt_constraint(0)
		, _bodyB(rbB)
	{
		_bt_constraint = new btHingeConstraint(*rbB->getNative(), 
			convert_bt_transform(frameInB), useLinearReferenceFrameB);
	}

	btHingeConstraint *_bt_constraint;

	RigidBodyWeakPtr _bodyA;
	RigidBodyWeakPtr _bodyB;

};	// class HingeConstraint

}	// namespace physics

}	// namepsace vl

#endif	// HYDRA_PHYSICS_CONSTRAINTS_HPP
