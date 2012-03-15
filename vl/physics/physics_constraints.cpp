/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-07
 *	@file physics/physics_constraints.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

/// Interface
#include "physics_constraints.hpp"

/// Concrete implementations
#ifdef USE_BULLET
#include "physics_constraints_bullet.hpp"
#else if USE_NEWTON
#include "physics_constraints_newton.hpp"
#endif

std::ostream &
vl::physics::operator<<(std::ostream &os, vl::physics::Constraint const &c)
{
	os << "Constraint between " << c.getBodyA()->getName() 
			<< " and " << c.getBodyA()->getName() << std::endl;

	return os;
}

std::ostream &
vl::physics::operator<<(std::ostream &os, vl::physics::HingeConstraint const &c)
{
	os << "Hinge constraint between " << c.getBodyA()->getName() 
			<< " and " << c.getBodyA()->getName() << std::endl;

	return os;
}

std::ostream &
vl::physics::operator<<(std::ostream &os, vl::physics::SliderConstraint const &c)
{
	os << "Slider constraint between " << c.getBodyA()->getName() 
			<< " and " << c.getBodyA()->getName() << std::endl;

	return os;
}

std::ostream &
vl::physics::operator<<(std::ostream &os, vl::physics::SixDofConstraint const &c)
{
	os << "Six Dof constraint between " << c.getBodyA()->getName() 
			<< " and " << c.getBodyA()->getName() << std::endl;

	return os;
}

std::ostream &
vl::physics::operator<<(std::ostream &os, vl::physics::ConstraintList const &cl)
{
	os << "Constraint list with " << cl.size() << " constraints." << std::endl;
	for(ConstraintList::const_iterator iter = cl.begin(); iter != cl.end(); ++iter)
	{
		os << " between " << (*iter)->getBodyA()->getName() 
			<< " and " << (*iter)->getBodyA()->getName() << std::endl;
	}

	return os;
}


vl::physics::SixDofConstraintRefPtr
vl::physics::SixDofConstraint::create(vl::physics::RigidBodyRefPtr rbA, vl::physics::RigidBodyRefPtr rbB, 
	Transform const &frameInA, Transform const &frameInB, bool useLinearReferenceFrameA)
{
	SixDofConstraintRefPtr constraint;
#ifdef USE_BULLET
	constraint.reset(new BulletSixDofConstraint(rbA, rbB, frameInA, frameInB, useLinearReferenceFrameA));
#else if USE_NEWTON
#endif
	return constraint;
}

vl::physics::SliderConstraintRefPtr
vl::physics::SliderConstraint::create(vl::physics::RigidBodyRefPtr rbA, vl::physics::RigidBodyRefPtr rbB, 
	Transform const &frameInA, Transform const &frameInB, bool useLinearReferenceFrameA)
{
	SliderConstraintRefPtr constraint;
#ifdef USE_BULLET
	constraint.reset(new BulletSliderConstraint(rbA, rbB, frameInA, frameInB, useLinearReferenceFrameA));
#else if USE_NEWTON
#endif
	return constraint;
}

vl::physics::HingeConstraintRefPtr
vl::physics::HingeConstraint::create(vl::physics::RigidBodyRefPtr rbA, vl::physics::RigidBodyRefPtr rbB, 
	Transform const &frameInA, Transform const &frameInB, bool useLinearReferenceFrameA)
{
	HingeConstraintRefPtr  constraint;
#ifdef USE_BULLET
	constraint.reset(new BulletHingeConstraint(rbA, rbB, frameInA, frameInB, useLinearReferenceFrameA));
#else if USE_NEWTON
#endif
	return constraint;
}
