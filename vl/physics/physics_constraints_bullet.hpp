/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-08
 *	@file physics/physics_constraints_bullet.hpp
 *
 *	This file is part of Hydra a VR game engine.
 */

#ifndef HYDRA_PHYSICS_CONSTRAINTS_BULLET_HPP
#define HYDRA_PHYSICS_CONSTRAINTS_BULLET_HPP

// Base classes
#include "physics_constraints.hpp"

// Concrete implementation
#include <bullet/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h>

#include "rigid_body_bullet.hpp"

namespace {
	using vl::math::convert_bt_vec;
	using vl::math::convert_vec;
	using vl::math::convert_bt_transform;
}

namespace vl
{

namespace physics
{

/// The base constraint for physics needs to be abstract interface
class BulletConstraint
{
public :
	virtual ~BulletConstraint(void) {}

	virtual btTypedConstraint *getNative(void) = 0;

};

typedef boost::shared_ptr<BulletConstraint> BulletConstraintRefPtr;

class BulletSixDofConstraint : public BulletConstraint, public SixDofConstraint
{
public :
	BulletSixDofConstraint(RigidBodyRefPtr rbA, RigidBodyRefPtr rbB, 
		Transform const &frameInA, Transform const &frameInB, bool useLinearReferenceFrameA)
		: SixDofConstraint(rbA, rbB, frameInA, frameInB, useLinearReferenceFrameA)
		, _bt_constraint(0)
	{
		assert(boost::dynamic_pointer_cast<BulletRigidBody>(rbA));
		assert(boost::dynamic_pointer_cast<BulletRigidBody>(rbB));
		BulletRigidBodyRefPtr body1 = boost::static_pointer_cast<BulletRigidBody>(rbA);
		BulletRigidBodyRefPtr body2 = boost::static_pointer_cast<BulletRigidBody>(rbB);

		_bt_constraint = new btGeneric6DofSpringConstraint(*body1->getNative(), 
			*body2->getNative(), convert_bt_transform(frameInA), 
			convert_bt_transform(frameInB), useLinearReferenceFrameA);
	}

	virtual ~BulletSixDofConstraint(void) {}

	void setLinearLowerLimit(Ogre::Vector3 const &linearLower)
	{ _bt_constraint->setLinearLowerLimit(convert_bt_vec(linearLower)); }

	void setLinearUpperLimit(Ogre::Vector3 const &linearUpper)
	{ _bt_constraint->setLinearUpperLimit(convert_bt_vec(linearUpper)); }

	void setAngularLowerLimit(Ogre::Vector3 const &angularLower)
	{ _bt_constraint->setAngularLowerLimit(convert_bt_vec(angularLower)); }

	void setAngularUpperLimit(Ogre::Vector3 const &angularUpper)
	{ _bt_constraint->setAngularUpperLimit(convert_bt_vec(angularUpper)); }

	void enableSpring(int index, bool onOff)
	{ _bt_constraint->enableSpring(index, onOff); }

	void setStiffness(int index, vl::scalar stiffness)
	{ _bt_constraint->setStiffness(index, stiffness); }

	void setDamping(int index, vl::scalar damping)
	{ _bt_constraint->setDamping(index, damping); }

	void setEquilibriumPoint(void)
	{ _bt_constraint->setEquilibriumPoint(); }

	void setEquilibriumPoint(int index)
	{ _bt_constraint->setEquilibriumPoint(index); }

	virtual btTypedConstraint *getNative(void)
	{ return _bt_constraint; }

private :
	btGeneric6DofSpringConstraint *_bt_constraint;

};

class BulletSliderConstraint : public BulletConstraint, public vl::physics::SliderConstraint
{
public :
	BulletSliderConstraint(RigidBodyRefPtr rbA, RigidBodyRefPtr rbB, 
		Transform const &frameInA, Transform const &frameInB, bool useLinearReferenceFrameA)
		: SliderConstraint(rbA, rbB, frameInA, frameInB, useLinearReferenceFrameA)
		, _bt_constraint(0)
	{
		assert(boost::dynamic_pointer_cast<BulletRigidBody>(rbA));
		assert(boost::dynamic_pointer_cast<BulletRigidBody>(rbB));
		BulletRigidBodyRefPtr body1 = boost::static_pointer_cast<BulletRigidBody>(rbA);
		BulletRigidBodyRefPtr body2 = boost::static_pointer_cast<BulletRigidBody>(rbB);

		_bt_constraint = new btSliderConstraint(*body1->getNative(), *body2->getNative(), 
			convert_bt_transform(frameInA), convert_bt_transform(frameInB), useLinearReferenceFrameA);
	}

	virtual ~BulletSliderConstraint(void) {}

	vl::scalar getLowerLinLimit(void) const
	{ return _bt_constraint->getLowerLinLimit(); }

	void setLowerLinLimit(vl::scalar lowerLimit)
	{ _bt_constraint->setLowerLinLimit(lowerLimit); }

	virtual vl::scalar getUpperLinLimit(void) const
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

	void setTargetLinMotorVelocity(vl::scalar targetLinMotorVelocity)
	{ _bt_constraint->setTargetLinMotorVelocity(targetLinMotorVelocity); }
	
	vl::scalar getTargetLinMotorVelocity(void)
	{ return _bt_constraint->getTargetLinMotorVelocity(); }

	void setMaxLinMotorForce(vl::scalar maxLinMotorForce)
	{ _bt_constraint->setMaxLinMotorForce(maxLinMotorForce); }

	virtual vl::scalar getMaxLinMotorForce(void)
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

	void setPoweredLinMotor(bool onOff)
	{ _bt_constraint->setPoweredLinMotor(onOff); }
	bool getPoweredLinMotor(void)
	{ return _bt_constraint->getPoweredLinMotor(); }

	virtual btTypedConstraint *getNative(void)
	{ return _bt_constraint; }

private :
	btSliderConstraint *_bt_constraint;
};

class BulletHingeConstraint : public BulletConstraint, public vl::physics::HingeConstraint
{
public :
	BulletHingeConstraint(RigidBodyRefPtr rbA, RigidBodyRefPtr rbB, 
		Transform const &frameInA, Transform const &frameInB, bool useLinearReferenceFrameA)
		: HingeConstraint(rbA, rbB, frameInA, frameInB, useLinearReferenceFrameA)
		, _bt_constraint(0)
	{
		assert(boost::dynamic_pointer_cast<BulletRigidBody>(rbA));
		assert(boost::dynamic_pointer_cast<BulletRigidBody>(rbB));
		BulletRigidBodyRefPtr body1 = boost::static_pointer_cast<BulletRigidBody>(rbA);
		BulletRigidBodyRefPtr body2 = boost::static_pointer_cast<BulletRigidBody>(rbB);

		_bt_constraint = new btHingeConstraint(*body1->getNative(), *body2->getNative(), 
			convert_bt_transform(frameInA), convert_bt_transform(frameInB), useLinearReferenceFrameA);
	}

	virtual ~BulletHingeConstraint(void) {}

	void setAngularOnly(bool angularOnly)
	{ _bt_constraint->setAngularOnly(angularOnly); }

	void enableAngularMotor(bool enableMotor, vl::scalar targetVelocity, vl::scalar maxMotorImpulse)
	{ _bt_constraint->enableAngularMotor(enableMotor, targetVelocity, maxMotorImpulse); }

	virtual void enableMotor(bool enableMotor)
	{ _bt_constraint->enableMotor(enableMotor); }
	
	void setMaxMotorImpulse(vl::scalar maxMotorImpulse)
	{ _bt_constraint->setMaxMotorImpulse(maxMotorImpulse); }

	void setMotorTarget(vl::scalar targetAngle, vl::scalar dt)
	{ _bt_constraint->setMotorTarget(targetAngle, dt); }
	void setLimit(vl::scalar low, vl::scalar high, vl::scalar softness=0.9f, vl::scalar biasFactor=0.3f, vl::scalar relaxationFactor=1.0f)
	{ _bt_constraint->setLimit(low, high, softness, biasFactor, relaxationFactor); }

	void setAxis(Ogre::Vector3 &axisInA)
	{
		_bt_constraint->setAxis(convert_bt_vec(axisInA));
	}

	vl::scalar getLowerLimit(void) const
	{ return _bt_constraint->getLowerLimit(); }

	vl::scalar getUpperLimit(void) const
	{ return _bt_constraint->getUpperLimit(); }

	vl::scalar getHingeAngle(void)
	{ return _bt_constraint->getHingeAngle(); }

	virtual btTypedConstraint *getNative(void)
	{ return _bt_constraint; }

private :
	btHingeConstraint *_bt_constraint;
};

}	// namespace physics

}	// namespace vl

#endif	// HYDRA_PHYSICS_CONSTRAINTS_BULLET_HPP
