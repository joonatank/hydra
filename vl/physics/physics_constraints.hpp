/*
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-05
 *	@file physics/physics_constraints.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

#ifndef HYDRA_PHYSICS_CONSTRAINTS_HPP
#define HYDRA_PHYSICS_CONSTRAINTS_HPP

#include "math/transform.hpp"
// Necessary for HYDRA_API
#include "defines.hpp"

#include "rigid_body.hpp"

namespace vl
{

namespace physics
{
//API (virtual) Class for rotational 3dof or translational 3dof joint motor
class Motor3Dof
{
public:
	//Constraint lowerlimit:
	virtual void			setLowerLimit(Ogre::Vector3 const& lowlimit)=0;
    virtual Ogre::Vector3	getLowerLimit(void)=0;
	
	//Constraint upper limits:
	virtual void			setUpperLimit(Ogre::Vector3 const&)=0;
    virtual Ogre::Vector3	getUpperLimit(void)=0;
	
	//Softness for limits:
    virtual void			setLimitSoftness(vl::scalar const&)=0;
	virtual vl::scalar		getLimitSoftness(void)=0;
	
	//Damping for limits:
    virtual void			setDamping(vl::scalar const&)=0;
	virtual vl::scalar		getDamping(void)=0;

	//Restitution parameter (0 = totally inelastic collision, 1 = totally elastic collision):
	virtual void			setRestitution(vl::scalar const&)=0;
	virtual vl::scalar		getRestitution(void)=0;
	
	//Normal constraint force mixing factor:
	virtual void			setNormalCFM(Ogre::Vector3 const&)=0;
    virtual Ogre::Vector3	getNormalCFM(void)=0;
	
	//Error tolerance factor when joint is at limit:
	virtual void			setStopERP(Ogre::Vector3 const&)=0;
    virtual Ogre::Vector3	getStopERP(void)=0;

	//Constraint force mixing factor when joint is at limit:
	virtual void			setStopCFM(Ogre::Vector3 const&)=0;
    virtual Ogre::Vector3	getStopCFM(void)=0;
    
	//Target motor velocity:
	virtual void			setTargetVelocity(Ogre::Vector3 const&)=0;
    virtual Ogre::Vector3	getTargetVelocity(void)=0;
	
    //Maximum force on motor, eg. maximum force used to achieve needed velocity:
	virtual void			setMaxMotorForce(Ogre::Vector3 const&)=0;
    virtual Ogre::Vector3	getMaxMotorForce(void)=0;	
	
	//Maximum returning torque when limit is violated (this is applied with rotational motors only):
	virtual void			setMaxLimitTorque(Ogre::Vector3 const&)=0;
    virtual	Ogre::Vector3	getMaxLimitTorque(void)=0;

	//Use the lock hack, defaults to which ever is working better
	//Do not change at run time only when creating the motor
	virtual void enableLocking(bool enable) = 0;
	virtual bool isLockingEnabled(void) = 0;

	//Is one of 3 dof's enabled:
	virtual void			enableMotor(int const)=0;
	virtual void			disableMotor(int const)=0;	
	virtual void			enableAllMotors(void)=0;
	virtual void			disableAllMotors(void)=0;
};

/// The base constraint for physics needs to be abstract interface
class Constraint
{
public :
	RigidBodyRefPtr getBodyA(void) const
	{ return _bodyA.lock(); }

	RigidBodyRefPtr getBodyB(void) const
	{ return _bodyB.lock(); }

	virtual Transform const &getLocalFrameA(void) const
	{ return _frameA; }

	virtual Transform const &getLocalFrameB(void) const
	{ return _frameB; }

	virtual std::string getTypeName(void) const = 0;

	std::string const &getName(void) const
	{ return _name; }

	void setName(std::string const &name)
	{ _name = name; }

	void reset(RigidBodyRefPtr rbA, RigidBodyRefPtr rbB, 
		Transform const &frameInA, Transform const &frameInB);

	bool isDynamic(void) const
	{ return _is_dynamic; }

	virtual ~Constraint(void) {}

protected :
	Constraint(RigidBodyRefPtr rbA, RigidBodyRefPtr rbB, 
		Transform const &frameInA, Transform const &frameInB, bool dynamic)
		: _bodyA(rbA)
		, _bodyB(rbB)
		, _frameA(frameInA)
		, _frameB(frameInB)
		, _is_dynamic(dynamic)
	{}

private :
	Constraint &operator=(Constraint const &);
	Constraint(Constraint const &);

	// Called from reset function to finilise the reseting
	virtual void _reseted(void) = 0;

	// @todo these probably shouldn't be weak pointers
	// we don't own them, but the bodies should never be destroyed before the
	// constraint has been destroyed.
	RigidBodyWeakPtr _bodyA;
	RigidBodyWeakPtr _bodyB;

	Transform _frameA;
	Transform _frameB;

	std::string _name;

	bool _is_dynamic;
};

/// @class SixDofConstraint
/// Generic six dof constraint with a spring damper system
class HYDRA_API SixDofConstraint : public vl::physics::Constraint
{
public :
	virtual ~SixDofConstraint(void) {}

	virtual Ogre::Vector3 getLinearLowerLimit(void) const = 0;
	virtual void setLinearLowerLimit(Ogre::Vector3 const &linearLower) = 0;
	
	virtual Ogre::Vector3 getLinearUpperLimit(void) const = 0;
	virtual void setLinearUpperLimit(Ogre::Vector3 const &linearUpper) = 0;

	virtual Ogre::Vector3 getAngularLowerLimit(void) const = 0;
	virtual void setAngularLowerLimit(Ogre::Vector3 const &angularLower) = 0;

	virtual Ogre::Vector3 getAngularUpperLimit(void) const = 0;
	virtual void setAngularUpperLimit(Ogre::Vector3 const &angularUpper) = 0;

	virtual Ogre::Vector3 getCurrentPosition(void) const = 0;
	virtual Ogre::Vector3 getCurrentAngle(void) const = 0;

	//These belong to constraint api, altough I didn't have
	//time to add it for everything so until now it's only added to
	//6 dof:
	virtual vl::Transform const  &getFrameOffsetA(void) const = 0;
	virtual vl::Transform const &getFrameOffsetB(void) const = 0;
	virtual void setFrameOffsetA(Transform const &) = 0;
	virtual void setFrameOffsetB(Transform const &) = 0;

	/// Index 0-2 for translation (x, y, z)
	/// Index 3-5 for rotations (x, y, z)
	virtual void enableSpring(int index, bool onOff) = 0;
	virtual void setStiffness(int index, vl::scalar stiffness) = 0;
	virtual void setDamping(int index, vl::scalar damping) = 0;
	virtual void setEquilibriumPoint(void) = 0;
	virtual void setEquilibriumPoint(int index) = 0;

	virtual void setNormalCFM(vl::scalar cfm) = 0;

	virtual void setStopCFM(vl::scalar cfm) = 0;

	virtual void setStopERP(vl::scalar erp) = 0;

	// @todo add motors
	//Ville lisännyt:
	//Get motors as pointers so you can straightaway edit those, no need for setter.
	virtual Motor3Dof *getTranslationalMotor(void) = 0;
	virtual Motor3Dof *getRotationalMotor(void) = 0;
	
	
	// overloads
	virtual std::string getTypeName(void) const
	{ return "6dof"; }

	// static methods
	static SixDofConstraintRefPtr create(RigidBodyRefPtr rbA, RigidBodyRefPtr rbB, 
		Transform const &frameInA, Transform const &frameInB, bool useLinearReferenceFrameA = true);

	static SixDofConstraintRefPtr createDynamic(RigidBodyRefPtr rbA, RigidBodyRefPtr rbB, 
		Transform const &frameInA, Transform const &frameInB, bool useLinearReferenceFrameA = true);

protected :
	static SixDofConstraintRefPtr _create(RigidBodyRefPtr rbA, RigidBodyRefPtr rbB, 
		Transform const &frameInA, Transform const &frameInB, bool useLinearReferenceFrameA, bool dynamic);

	SixDofConstraint(RigidBodyRefPtr rbA, RigidBodyRefPtr rbB, 
		Transform const &frameInA, Transform const &frameInB, bool useLinearReferenceFrameA, bool dynamic)
		: Constraint(rbA, rbB, frameInA, frameInB, dynamic)
	{}

};	// class SixDofConstraint

class HYDRA_API SliderConstraint : public vl::physics::Constraint
{
public :
	virtual ~SliderConstraint(void) {}

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

	// overloads
	virtual std::string getTypeName(void) const
	{ return "slider"; }

	// static
	static SliderConstraintRefPtr create(RigidBodyRefPtr rbA, RigidBodyRefPtr rbB, 
		Transform const &frameInA, Transform const &frameInB, bool useLinearReferenceFrameA = true);
		
	static SliderConstraintRefPtr createDynamic(RigidBodyRefPtr rbA, RigidBodyRefPtr rbB, 
		Transform const &frameInA, Transform const &frameInB, bool useLinearReferenceFrameA = true);

protected :
	static SliderConstraintRefPtr _create(RigidBodyRefPtr rbA, RigidBodyRefPtr rbB, 
		Transform const &frameInA, Transform const &frameInB, bool useLinearReferenceFrameA, bool dynamic);

	SliderConstraint(RigidBodyRefPtr rbA, RigidBodyRefPtr rbB, 
		Transform const &frameInA, Transform const &frameInB, bool useLinearReferenceFrameA, bool dynamic)
		: Constraint(rbA, rbB, frameInA, frameInB, dynamic)
	{}

};	// class SliderConstraint

class HYDRA_API HingeConstraint : public vl::physics::Constraint
{
public :
	virtual ~HingeConstraint(void) {}

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
	
	// overloads
	virtual std::string getTypeName(void) const
	{ return "hinge"; }

	// static
	static HingeConstraintRefPtr create(RigidBodyRefPtr rbA, RigidBodyRefPtr rbB, 
		Transform const &frameInA, Transform const &frameInB, bool useLinearReferenceFrameA = true);
	
	static HingeConstraintRefPtr createDynamic(RigidBodyRefPtr rbA, RigidBodyRefPtr rbB, 
		Transform const &frameInA, Transform const &frameInB, bool useLinearReferenceFrameA = true);

protected :
	static HingeConstraintRefPtr _create(RigidBodyRefPtr rbA, RigidBodyRefPtr rbB, 
		Transform const &frameInA, Transform const &frameInB, bool useLinearReferenceFrameA, bool dynamic);

	HingeConstraint(RigidBodyRefPtr rbA, RigidBodyRefPtr rbB, 
		Transform const &frameInA, Transform const &frameInB, bool useLinearReferenceFrameA, bool dynamic)
		: Constraint(rbA, rbB, frameInA, frameInB, dynamic)
	{}

};	// class HingeConstraint


std::ostream &operator<<(std::ostream &os, Constraint const &c);
std::ostream &operator<<(std::ostream &os, HingeConstraint const &c);
std::ostream &operator<<(std::ostream &os, SliderConstraint const &c);
std::ostream &operator<<(std::ostream &os, SixDofConstraint const &c);

std::ostream &operator<<(std::ostream &os, ConstraintList const &cl);


}	// namespace physics

}	// namepsace vl

#endif	// HYDRA_PHYSICS_CONSTRAINTS_HPP
