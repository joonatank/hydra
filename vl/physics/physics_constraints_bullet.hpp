/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-08
 *	@file physics/physics_constraints_bullet.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
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
// Interfaces for 3dof rotational and translational motors, both inherited from Motor3Dof.
// Warning: user never should create these types by it's own, one always should retrieve pointer to this type from Generic6Dof constraint:
class Motor3DofTranslational : public Motor3Dof
{
public:
	//Constructor
	Motor3DofTranslational(btTranslationalLimitMotor* input) {
	mot=input;
	}

	//Constraint lowerlimit:
	virtual void			setLowerLimit(Ogre::Vector3 const& limit) {
		mot->m_lowerLimit = convert_bt_vec(limit);
	}
    virtual Ogre::Vector3	getLowerLimit(void) {
		return convert_vec(mot->m_lowerLimit);
	}
	
	//Constraint upper limits:
	virtual void			setUpperLimit(Ogre::Vector3 const& limit) {
		mot->m_upperLimit = convert_bt_vec(limit);
	}
    virtual Ogre::Vector3	getUpperLimit(void) {
		return convert_vec(mot->m_upperLimit);
	}
	
	//Softness for limits:
    virtual void			setLimitSoftness(vl::scalar const& soft) {
		mot->m_limitSoftness = soft;
	}
	virtual vl::scalar		getLimitSoftness(void) {
		return mot->m_limitSoftness;
	}
	
	//Damping for limits:
    virtual void			setDamping(vl::scalar const& damp) {
		mot->m_damping = damp;
	}
	virtual vl::scalar		getDamping(void) {
		return mot->m_damping;
	}

	//Restitution parameter (0 = totally inelastic collision, 1 = totally elastic collision):
	virtual void			setRestitution(vl::scalar const& restitution) {
		mot->m_restitution = restitution;
	}
	virtual vl::scalar		getRestitution(void) {
		return mot->m_restitution;
	}
	
	//Normal constraint force mixing factor:
	virtual void			setNormalCFM(Ogre::Vector3 const& ncfm) {
		mot->m_normalCFM = convert_bt_vec(ncfm);
	}
    virtual Ogre::Vector3	getNormalCFM(void) {
		return convert_vec(mot->m_normalCFM);
	}
	//Error tolerance factor when joint is at limit:
	virtual void			setStopERP(Ogre::Vector3 const& serp) {
		mot->m_stopERP = convert_bt_vec(serp);
	}
	virtual Ogre::Vector3	getStopERP(void) {
		return convert_vec(mot->m_stopERP);
	}
	//Constraint force mixing factor when joint is at limit:
	virtual void			setStopCFM(Ogre::Vector3 const& scfm) {
		mot->m_stopCFM = convert_bt_vec(scfm);
	}
	virtual Ogre::Vector3	getStopCFM(void) {
		return convert_vec(mot->m_stopCFM);
	}    
	//Target motor velocity:
	virtual void			setTargetVelocity(Ogre::Vector3 const& vel) {
		mot->m_targetVelocity = convert_bt_vec(vel);
	}
	virtual Ogre::Vector3	getTargetVelocity(void) {
		return convert_vec(mot->m_targetVelocity);
	}
	
    //Maximum force on motor, eg. maximum force used to achieve needed velocity:
	virtual void			setMaxMotorForce(Ogre::Vector3 const& force) {
		mot->m_maxMotorForce = convert_bt_vec(force);
	}
	virtual Ogre::Vector3	getMaxMotorForce(void) {
		return convert_vec(mot->m_maxMotorForce);
	}	
	//Maximum returning torque when limit is violated (this is applied with rotational motors only):
	virtual void			setMaxLimitTorque(Ogre::Vector3 const& torq) {
		
	}
	virtual	Ogre::Vector3	getMaxLimitTorque(void) {
		return Ogre::Vector3::ZERO; 
	}
	//Is one of 3 dof's enabled:
	virtual void			enableMotor(int const index) {
		if(index == 0 || index == 1 || index == 2) {
			mot->m_enableMotor[index] = true;
		}
		else {
		//@TODO: add error report
		}
	}
	virtual void			disableMotor(int const index) {
		if(index == 0 || index == 1 || index == 2) {
			mot->m_enableMotor[index] = false;
		}
		else {
		//@TODO: add error report
		}
		
	}
	
	virtual void			enableAllMotors(void) {
		mot->m_enableMotor[0] = true;
		mot->m_enableMotor[1] = true;
		mot->m_enableMotor[2] = true;
	}
	virtual void			disableAllMotors(void) {
		mot->m_enableMotor[0] = false;
		mot->m_enableMotor[1] = false;
		mot->m_enableMotor[2] = false;
		
	}
private:
	btTranslationalLimitMotor* mot;
};
class Motor3DofRotational : public Motor3Dof
{
public:
	//Constructor
	Motor3DofRotational(std::vector<btRotationalLimitMotor*> input) {
		mot=input;
		assert(mot.size() == 3);
	}

	//Constraint lowerlimit:
	virtual void			setLowerLimit(Ogre::Vector3 const& limit) {
		mot.at(0)->m_loLimit = (convert_bt_vec(limit)).getX();
		mot.at(1)->m_loLimit = (convert_bt_vec(limit)).getY();
		mot.at(2)->m_loLimit = (convert_bt_vec(limit)).getZ();
	}
    virtual Ogre::Vector3	getLowerLimit(void) {
		return convert_vec(btVector3(mot.at(0)->m_loLimit, mot.at(1)->m_loLimit, mot.at(2)->m_loLimit));
	}
	
	//Constraint upper limits:
	virtual void			setUpperLimit(Ogre::Vector3 const& limit) {
		mot.at(0)->m_hiLimit = (convert_bt_vec(limit)).getX();
		mot.at(1)->m_hiLimit = (convert_bt_vec(limit)).getY();
		mot.at(2)->m_hiLimit = (convert_bt_vec(limit)).getZ();
	}
    virtual Ogre::Vector3	getUpperLimit(void) {
		return convert_vec(btVector3(mot.at(0)->m_hiLimit, mot.at(1)->m_hiLimit, mot.at(2)->m_hiLimit));
	}
	
	//Softness for limits, , it's scalar on bullet documentation:
    virtual void			setLimitSoftness(vl::scalar const& soft) {
		mot.at(0)->m_limitSoftness = soft;
		mot.at(1)->m_limitSoftness = soft;
		mot.at(2)->m_limitSoftness = soft;
	}
	virtual vl::scalar		getLimitSoftness(void) {
		return mot.at(0)->m_limitSoftness;
	}
	
	//Damping for limits, it's scalar on bullet documentation:
    virtual void			setDamping(vl::scalar const& damp) {
		mot.at(0)->m_damping = damp;
		mot.at(1)->m_damping = damp;
		mot.at(2)->m_damping = damp;
	}
	virtual vl::scalar		getDamping(void) {
		return mot.at(0)->m_damping;
	}

	//Restitution parameter (0 = totally inelastic collision, 1 = totally elastic collision),
	//it's scalar on bullet documentation:
	virtual void			setRestitution(vl::scalar const& restitution) {
		mot.at(0)->m_bounce = restitution;
		mot.at(1)->m_bounce = restitution;
		mot.at(2)->m_bounce = restitution;
	}
	virtual vl::scalar		getRestitution(void) {
		return mot.at(0)->m_bounce;
	}
	
	//Normal constraint force mixing factor:
	virtual void			setNormalCFM(Ogre::Vector3 const& ncfm) {
		mot.at(0)->m_normalCFM = (convert_bt_vec(ncfm)).getX();
		mot.at(1)->m_normalCFM = (convert_bt_vec(ncfm)).getY();
		mot.at(2)->m_normalCFM = (convert_bt_vec(ncfm)).getZ();
	}
    virtual Ogre::Vector3	getNormalCFM(void) {
		return convert_vec(btVector3(mot.at(0)->m_normalCFM,mot.at(1)->m_normalCFM,mot.at(2)->m_normalCFM));
	}
	//Error tolerance factor when joint is at limit:
	virtual void			setStopERP(Ogre::Vector3 const& serp) {
		mot.at(0)->m_stopERP = (convert_bt_vec(serp)).getX();
		mot.at(1)->m_stopERP = (convert_bt_vec(serp)).getY();
		mot.at(2)->m_stopERP = (convert_bt_vec(serp)).getZ();
	}
	virtual Ogre::Vector3	getStopERP(void) {
		return convert_vec(btVector3(mot.at(0)->m_stopERP,mot.at(1)->m_stopERP,mot.at(2)->m_stopERP));
	}
	//Constraint force mixing factor when joint is at limit:
	virtual void			setStopCFM(Ogre::Vector3 const& scfm) {
		mot.at(0)->m_stopCFM = (convert_bt_vec(scfm)).getX();
		mot.at(1)->m_stopCFM = (convert_bt_vec(scfm)).getY();
		mot.at(2)->m_stopCFM = (convert_bt_vec(scfm)).getZ();
	}
	virtual Ogre::Vector3	getStopCFM(void) {
		return convert_vec(btVector3(mot.at(0)->m_stopCFM,mot.at(1)->m_stopCFM,mot.at(2)->m_stopCFM));
	}    
	//Target motor velocity:
	virtual void			setTargetVelocity(Ogre::Vector3 const& vel) {
		mot.at(0)->m_targetVelocity = (convert_bt_vec(vel)).getX();
		mot.at(1)->m_targetVelocity = (convert_bt_vec(vel)).getY();
		mot.at(2)->m_targetVelocity = (convert_bt_vec(vel)).getZ();
	}
	virtual Ogre::Vector3	getTargetVelocity(void) {
		return convert_vec(btVector3(mot.at(0)->m_targetVelocity,mot.at(1)->m_targetVelocity,mot.at(2)->m_targetVelocity));
	}
	
    //Maximum force on motor, eg. maximum force used to achieve needed velocity:
	virtual void			setMaxMotorForce(Ogre::Vector3 const& force) {
		mot.at(0)->m_maxMotorForce = convert_bt_vec(force).getX();
		mot.at(1)->m_maxMotorForce = convert_bt_vec(force).getY();
		mot.at(2)->m_maxMotorForce = convert_bt_vec(force).getZ();

	}
	virtual Ogre::Vector3	getMaxMotorForce(void) {
		return convert_vec(btVector3(mot.at(0)->m_maxMotorForce,
			mot.at(1)->m_maxMotorForce,
			mot.at(2)->m_maxMotorForce));
	}	
	//Maximum returning torque when limit is violated (this is applied with rotational motors only):
	virtual void			setMaxLimitTorque(Ogre::Vector3 const& torq) {
		mot.at(0)->m_maxMotorForce = (convert_bt_vec(torq)).getX();
		mot.at(1)->m_maxMotorForce = (convert_bt_vec(torq)).getY();
		mot.at(2)->m_maxMotorForce = (convert_bt_vec(torq)).getZ();
	}
	virtual	Ogre::Vector3	getMaxLimitTorque(void) {
		return convert_vec(btVector3(mot.at(0)->m_maxMotorForce,
			mot.at(1)->m_maxMotorForce,
			mot.at(2)->m_maxMotorForce));
	}
	//Disabling and enabling motors:
	virtual void			enableMotor(int const index) {		
		if(index == 0 || index == 1 || index == 2) {
			mot[index]->m_enableMotor = true;
		}
		else {
		//@TODO: add error report
		}
	}
	virtual void			disableMotor(int const index) {
		if(index == 0 || index == 1 || index == 2) {
			mot[index]->m_enableMotor = false;
		}
		else {
		//@TODO: add error report
		}
		
	}
	virtual void			enableAllMotors(void) {
		mot.at(0)->m_enableMotor = true;
		mot.at(1)->m_enableMotor = true;
		mot.at(2)->m_enableMotor = true;
	}
	virtual void			disableAllMotors(void) {
		mot.at(0)->m_enableMotor = false;
		mot.at(1)->m_enableMotor = false;
		mot.at(2)->m_enableMotor = false;
	}

private: 
	std::vector<btRotationalLimitMotor*>  mot;
};

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
		
		_transmot = new Motor3DofTranslational(_bt_constraint->getTranslationalLimitMotor());
		
		std::vector<btRotationalLimitMotor*> tmpvec;
		tmpvec.push_back(_bt_constraint->getRotationalLimitMotor(0));
		tmpvec.push_back(_bt_constraint->getRotationalLimitMotor(1));
		tmpvec.push_back(_bt_constraint->getRotationalLimitMotor(2));
		_rotmot = new Motor3DofRotational(tmpvec);
	}

	virtual ~BulletSixDofConstraint(void) {
	
	}

	virtual Ogre::Vector3 getLinearLowerLimit(void) const
	{ return convert_vec(_bt_constraint->getTranslationalLimitMotor()->m_lowerLimit); }

	void setLinearLowerLimit(Ogre::Vector3 const &linearLower)
	{ _bt_constraint->setLinearLowerLimit(convert_bt_vec(linearLower)); }

	virtual Ogre::Vector3 getLinearUpperLimit(void) const
	{ return convert_vec(_bt_constraint->getTranslationalLimitMotor()->m_upperLimit); }

	void setLinearUpperLimit(Ogre::Vector3 const &linearUpper)
	{ _bt_constraint->setLinearUpperLimit(convert_bt_vec(linearUpper)); }

	virtual Ogre::Vector3 getAngularLowerLimit(void) const
	{
		vl::scalar x = _bt_constraint->getRotationalLimitMotor(0)->m_loLimit;
		vl::scalar y = _bt_constraint->getRotationalLimitMotor(1)->m_loLimit;
		vl::scalar z = _bt_constraint->getRotationalLimitMotor(2)->m_loLimit;
		return Ogre::Vector3(x, y, z);
	}

	void setAngularLowerLimit(Ogre::Vector3 const &angularLower)
	{ _bt_constraint->setAngularLowerLimit(convert_bt_vec(angularLower)); }

	virtual Ogre::Vector3 getAngularUpperLimit(void) const
	{
		vl::scalar x = _bt_constraint->getRotationalLimitMotor(0)->m_hiLimit;
		vl::scalar y = _bt_constraint->getRotationalLimitMotor(1)->m_hiLimit;
		vl::scalar z = _bt_constraint->getRotationalLimitMotor(2)->m_hiLimit;
		return Ogre::Vector3(x, y, z);
	}

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

	virtual void setNormalCFM(vl::scalar cfm)
	{
		for(size_t i = 0; i < 6; ++i)
		{
			_bt_constraint->setParam(BT_6DOF_FLAGS_CFM_NORM, cfm, i);
		}
	}

	virtual void setStopCFM(vl::scalar cfm)
	{
		for(size_t i = 0; i < 6; ++i)
		{
			_bt_constraint->setParam(BT_CONSTRAINT_STOP_CFM, cfm, i);
		}
	}

	virtual void setStopERP(vl::scalar erp)
	{
		for(size_t i = 0; i < 6; ++i)
		{
			_bt_constraint->setParam(BT_6DOF_FLAGS_ERP_STOP, erp, i);
		}
	}

	virtual btTypedConstraint *getNative(void)
	{ return _bt_constraint; }
	//@todo: LISÄÄ GETTERI JA SETTERI TÄHÄN PRIVAMUUTTUJILLE!
	
	//Motor stuff here, converter to own struct
	//@todo: RETURNAA POINTTERIT NÄISTÄ, KUN PRIVAMUUTTUJAT ON KUNNOSSA:
	Motor3Dof *getTranslationalMotor(void) {
		return _transmot;
	}
	Motor3Dof *getRotationalMotor(void) {
		return _rotmot;
	}

private :
	btGeneric6DofSpringConstraint *_bt_constraint;
	
	Motor3DofTranslational *_transmot;
	Motor3DofRotational *_rotmot;
	
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
	void setLimit(Ogre::Radian const &low, Ogre::Radian const &high, vl::scalar softness=0.9f, vl::scalar biasFactor=0.3f, vl::scalar relaxationFactor=1.0f)
	{ _bt_constraint->setLimit(low.valueRadians(), high.valueRadians(), softness, biasFactor, relaxationFactor); }

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
