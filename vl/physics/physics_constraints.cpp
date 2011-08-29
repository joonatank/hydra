/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-07
 *	@file physics/physics_constraints.cpp
 *
 *	This file is part of Hydra a VR game engine.
 */

/// Interface
#include "physics_constraints.hpp"

/// Concrete implementations
#ifdef USE_BULLET
#include "physics_constraints_bullet.hpp"
#else if USE_NEWTON
#include "physics_constraints_newton.hpp"
#endif

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
