/**
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-07
 *	@file physics/physics_constraints.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
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
	std::string name1, name2;
	if(c.getBodyA())
	{ name1 = c.getBodyA()->getName(); }
	if(c.getBodyB())
	{ name2 = c.getBodyB()->getName(); }
	
	// @todo if there is no body we should print so
	os << "Constraint between " << name1 << " and " << name2 << std::endl;

	return os;
}

std::ostream &
vl::physics::operator<<(std::ostream &os, vl::physics::HingeConstraint const &c)
{
	std::string name1, name2;
	if(c.getBodyA())
	{ name1 = c.getBodyA()->getName(); }
	if(c.getBodyB())
	{ name2 = c.getBodyB()->getName(); }
	
	// @todo if there is no body we should print so
	os << "Hinge constraint between " << name1 << " and " << name2 << std::endl;

	return os;
}

std::ostream &
vl::physics::operator<<(std::ostream &os, vl::physics::SliderConstraint const &c)
{
	std::string name1, name2;
	if(c.getBodyA())
	{ name1 = c.getBodyA()->getName(); }
	if(c.getBodyB())
	{ name2 = c.getBodyB()->getName(); }
	
	// @todo if there is no body we should print so
	os << "Slider constraint between " << name1 << " and " << name2 << std::endl;

	return os;
}

std::ostream &
vl::physics::operator<<(std::ostream &os, vl::physics::SixDofConstraint const &c)
{
	std::string name1, name2;
	if(c.getBodyA())
	{ name1 = c.getBodyA()->getName(); }
	if(c.getBodyB())
	{ name2 = c.getBodyB()->getName(); }
	
	// @todo if there is no body we should print so
	os << "Six dof constraint between " << name1 << " and " << name2 << std::endl;

	return os;
}

std::ostream &
vl::physics::operator<<(std::ostream &os, vl::physics::ConstraintList const &cl)
{
	os << "Constraint list with " << cl.size() << " constraints." << std::endl;
	for(ConstraintList::const_iterator iter = cl.begin(); iter != cl.end(); ++iter)
	{
		os << **iter << std::endl;
	}

	return os;
}

void
vl::physics::Constraint::reset(RigidBodyRefPtr rbA, RigidBodyRefPtr rbB, 
	Transform const &frameInA, Transform const &frameInB)
{
	_bodyA = rbA;
	_bodyB = rbB;
	_frameA = frameInA;
	_frameB = frameInB;

	_reseted();
}


vl::physics::SixDofConstraintRefPtr
vl::physics::SixDofConstraint::create(vl::physics::RigidBodyRefPtr rbA, vl::physics::RigidBodyRefPtr rbB, 
	Transform const &frameInA, Transform const &frameInB, bool useLinearReferenceFrameA)
{
	return _create(rbA, rbB, frameInA, frameInB, useLinearReferenceFrameA, false);
}

vl::physics::SixDofConstraintRefPtr
vl::physics::SixDofConstraint::createDynamic(vl::physics::RigidBodyRefPtr rbA, vl::physics::RigidBodyRefPtr rbB, 
	Transform const &frameInA, Transform const &frameInB, bool useLinearReferenceFrameA)
{
	return _create(rbA, rbB, frameInA, frameInB, useLinearReferenceFrameA, true);
}

vl::physics::SixDofConstraintRefPtr
vl::physics::SixDofConstraint::_create(vl::physics::RigidBodyRefPtr rbA, vl::physics::RigidBodyRefPtr rbB, 
	Transform const &frameInA, Transform const &frameInB, bool useLinearReferenceFrameA, bool dynamic)
{
	SixDofConstraintRefPtr constraint;
#ifdef USE_BULLET
	constraint.reset(new BulletSixDofConstraint(rbA, rbB, frameInA, frameInB, useLinearReferenceFrameA, dynamic));
#else if USE_NEWTON
#endif
	return constraint;
}

vl::physics::SliderConstraintRefPtr
vl::physics::SliderConstraint::create(vl::physics::RigidBodyRefPtr rbA, vl::physics::RigidBodyRefPtr rbB, 
	Transform const &frameInA, Transform const &frameInB, bool useLinearReferenceFrameA)
{
	return _create(rbA, rbB, frameInA, frameInB, useLinearReferenceFrameA, false);
}

vl::physics::SliderConstraintRefPtr
vl::physics::SliderConstraint::createDynamic(vl::physics::RigidBodyRefPtr rbA, vl::physics::RigidBodyRefPtr rbB, 
	Transform const &frameInA, Transform const &frameInB, bool useLinearReferenceFrameA)
{
	return _create(rbA, rbB, frameInA, frameInB, useLinearReferenceFrameA, true);
}

vl::physics::SliderConstraintRefPtr
vl::physics::SliderConstraint::_create(vl::physics::RigidBodyRefPtr rbA, vl::physics::RigidBodyRefPtr rbB, 
	Transform const &frameInA, Transform const &frameInB, bool useLinearReferenceFrameA, bool dynamic)
{
	SliderConstraintRefPtr constraint;
#ifdef USE_BULLET
	constraint.reset(new BulletSliderConstraint(rbA, rbB, frameInA, frameInB, useLinearReferenceFrameA, dynamic));
#else if USE_NEWTON
#endif
	return constraint;
}

vl::physics::HingeConstraintRefPtr
vl::physics::HingeConstraint::create(vl::physics::RigidBodyRefPtr rbA, vl::physics::RigidBodyRefPtr rbB, 
	Transform const &frameInA, Transform const &frameInB, bool useLinearReferenceFrameA)
{
	return _create(rbA, rbB, frameInA, frameInB, useLinearReferenceFrameA, false);
}

vl::physics::HingeConstraintRefPtr
vl::physics::HingeConstraint::createDynamic(vl::physics::RigidBodyRefPtr rbA, vl::physics::RigidBodyRefPtr rbB, 
	Transform const &frameInA, Transform const &frameInB, bool useLinearReferenceFrameA)
{
	return _create(rbA, rbB, frameInA, frameInB, useLinearReferenceFrameA, true);
}

vl::physics::HingeConstraintRefPtr
vl::physics::HingeConstraint::_create(RigidBodyRefPtr rbA, RigidBodyRefPtr rbB, 
	Transform const &frameInA, Transform const &frameInB, bool useLinearReferenceFrameA, bool dynamic)
{
	HingeConstraintRefPtr  constraint;
#ifdef USE_BULLET
	constraint.reset(new BulletHingeConstraint(rbA, rbB, frameInA, frameInB, useLinearReferenceFrameA, dynamic));
#else if USE_NEWTON
#endif
	return constraint;
}
