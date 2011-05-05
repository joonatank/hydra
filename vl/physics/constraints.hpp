/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-05
 *	@file constraints.hpp
 */

#ifndef HYDRA_PHYSICS_CONSTRAINTS_HPP
#define HYDRA_PHYSICS_CONSTRAINTS_HPP

#include "math/transform.hpp"

#include "rigid_body.hpp"

#include <bullet/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h>

namespace {
	using vl::math::convert_bt_vec;
	using vl::math::convert_vec;
	using vl::math::convert_bt_transform;
}

namespace vl
{

namespace physics
{

class SixDofConstraint
{
public :
	SixDofConstraint(RigidBodyRefPtr rbA, RigidBodyRefPtr rbB, 
		Transform const &frameInA, Transform const &frameInB, bool useLinearReferenceFrameA)
		: _bt_constraint(0)
		, _bodyA(rbA)
		, _bodyB(rbB)
	{
		_bt_constraint = new btGeneric6DofConstraint(*rbA->getNative(), *rbB->getNative(), convert_bt_transform(frameInA), convert_bt_transform(frameInB), useLinearReferenceFrameA);
	}

 	SixDofConstraint(RigidBodyRefPtr rbB, Transform const &frameInB, bool useLinearReferenceFrameB)
		: _bt_constraint(0)
		, _bodyB(rbB)
	{
		_bt_constraint = new btGeneric6DofConstraint(*rbB->getNative(), convert_bt_transform(frameInB), useLinearReferenceFrameB);
	}

	void setLinearLowerLimit(Ogre::Vector3 const &linearLower)
	{
		assert(_bt_constraint);
		_bt_constraint->setLinearLowerLimit(convert_bt_vec(linearLower));
	}
	
	void setLinearUpperLimit(Ogre::Vector3 const &linearUpper)
	{
		assert(_bt_constraint);
		_bt_constraint->setLinearUpperLimit(convert_bt_vec(linearUpper));
	}

	void setAngularLowerLimit(Ogre::Vector3 const &angularLower)
	{
		assert(_bt_constraint);
		_bt_constraint->setAngularLowerLimit(convert_bt_vec(angularLower));
	}

	void setAngularUpperLimit(Ogre::Vector3 const &angularUpper)
	{
		assert(_bt_constraint);
		_bt_constraint->setAngularUpperLimit(convert_bt_vec(angularUpper));
	}

	RigidBodyRefPtr getBodyA(void)
	{ return _bodyA.lock(); }

	RigidBodyRefPtr getBodyB(void)
	{ return _bodyB.lock(); }

private :
	btGeneric6DofConstraint *_bt_constraint;

	RigidBodyWeakPtr _bodyA;
	RigidBodyWeakPtr _bodyB;

};	// class SixDofConstraint

}	// namespace physics

}	// namepsace vl

#endif	// HYDRA_PHYSICS_CONSTRAINTS_HPP