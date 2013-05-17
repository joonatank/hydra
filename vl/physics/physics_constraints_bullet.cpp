/**
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-08
 *	@file physics/physics_constraints_bullet.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

// Header
#include "physics_constraints_bullet.hpp"

namespace {
	using vl::math::convert_bt_vec;
	using vl::math::convert_vec;
	using vl::math::convert_bt_transform;
}

// Motor Implementation
void
vl::physics::Motor3DofTranslational::setTargetVelocity(Ogre::Vector3 const& vel)
{
	_mot->m_targetVelocity = convert_bt_vec(vel);
	
	if(_lock_hack)
	{
		// Hack to lock the constraint when velocity is zero
		// @todo this needs to be per axis check
		if(vl::equal(vel, Ogre::Vector3::ZERO))
		{
			for(size_t i = 0; i < 3; ++i)
			{
				_mot->m_enableMotor[i] = false;
				_locked[i] = true;
			}

			// should this use upper/lower limit or is position fine?
			Ogre::Vector3 p = _constraint->getCurrentPosition();
			// save old values
			_lower_limit = _mot->m_lowerLimit;
			_upper_limit = _mot->m_upperLimit;
			_constraint->setLinearLowerLimit(p);
			_constraint->setLinearUpperLimit(p);
		}
		else
		{
			bool locked = false;
			// If the constraint is locked we need to break the lock
			for(size_t i = 0; i < 3; ++i)
			{
				if(_locked[i])
				{
					locked = true;
					_mot->m_enableMotor[i] = true;
					_locked[i] = false;
				}
			}

			if(locked)
			{
				_mot->m_lowerLimit = _lower_limit;
				_mot->m_upperLimit = _upper_limit;
			}
		}
	}
	
}

void
vl::physics::Motor3DofRotational::setTargetVelocity(Ogre::Vector3 const& vel) 
{
	_mot.at(0)->m_targetVelocity = vel.x;
	_mot.at(1)->m_targetVelocity = vel.y;
	_mot.at(2)->m_targetVelocity = vel.z;
	
	if(_lock_hack)
	{
		// Hack to lock the constraint when velocity is zero
		// @todo this needs to be per axis check
		if(vl::equal(vel, Ogre::Vector3::ZERO))
		{
			for(size_t i = 0; i < 3; ++i)
			{
				_mot[i]->m_enableMotor = false;
				_locked[i] = true;
			}

			// should this use upper/lower limit or is position fine?
			Ogre::Vector3 p = _constraint->getCurrentAngle();
			// save old values
			_lower_limit = btVector3(_mot[0]->m_loLimit, _mot[1]->m_loLimit, _mot[2]->m_loLimit);
			_upper_limit = btVector3(_mot[0]->m_hiLimit, _mot[1]->m_hiLimit, _mot[2]->m_hiLimit);
			_constraint->setAngularLowerLimit(p);
			_constraint->setAngularUpperLimit(p);
		}
		else
		{
			bool locked = false;
			// If the constraint is locked we need to break the lock
			for(size_t i = 0; i < 3; ++i)
			{
				if(_locked[i])
				{
					locked = true;
					_mot[i]->m_enableMotor = true;
					_locked[i] = false;
				}
			}

			if(locked)
			{
				_mot[0]->m_hiLimit = _upper_limit.getX();
				_mot[1]->m_hiLimit = _upper_limit.getY();
				_mot[2]->m_hiLimit = _upper_limit.getZ();
			
				_mot[0]->m_loLimit = _lower_limit.getX();
				_mot[1]->m_loLimit = _lower_limit.getY();
				_mot[2]->m_loLimit = _lower_limit.getZ();
			}
		}
	}
	
}

/// ------------------------------ BulletSixDofConstraint --------------------
vl::physics::BulletSixDofConstraint::BulletSixDofConstraint(RigidBodyRefPtr rbA, 
	RigidBodyRefPtr rbB, Transform const &frameInA, Transform const &frameInB, 
	bool useLinearReferenceFrameA, bool dynamic)
	: SixDofConstraint(rbA, rbB, frameInA, frameInB, useLinearReferenceFrameA, dynamic)
	, _bt_constraint(0)
{
	assert(boost::dynamic_pointer_cast<BulletRigidBody>(rbA));
	assert(boost::dynamic_pointer_cast<BulletRigidBody>(rbB));
	BulletRigidBodyRefPtr body1 = boost::static_pointer_cast<BulletRigidBody>(rbA);
	BulletRigidBodyRefPtr body2 = boost::static_pointer_cast<BulletRigidBody>(rbB);
		
	_bt_constraint = new btGeneric6DofSpringConstraint(*body1->getNative(), 
		*body2->getNative(), convert_bt_transform(frameInA), 
		convert_bt_transform(frameInB), useLinearReferenceFrameA);
		
	_transmot = new Motor3DofTranslational(_bt_constraint->getTranslationalLimitMotor(), this);

	std::vector<btRotationalLimitMotor*> tmpvec;
	tmpvec.push_back(_bt_constraint->getRotationalLimitMotor(0));
	tmpvec.push_back(_bt_constraint->getRotationalLimitMotor(1));
	tmpvec.push_back(_bt_constraint->getRotationalLimitMotor(2));
	_rotmot = new Motor3DofRotational(tmpvec, this);
}
