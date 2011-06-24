/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-06
 *	@file constraints.hpp
 *
 *	This file is part of Hydra a VR game engine.
 *
 *	Defines basic non-physical constraints. These are based on the common
 *	constraints in physics engines, but do not do the dynamics simulation.
 */

#ifndef HYDRA_CONSTRAINTS_HPP
#define HYDRA_CONSTRAINTS_HPP

#include "typedefs.hpp"

#include "math/math.hpp"
#include "math/transform.hpp"

#include "base/timer.hpp"

#include "animation.hpp"

namespace vl
{

class Constraint
{
public :
	virtual ~Constraint(void)
	{}

	SceneNodePtr getBodyA(void)
	{ return _bodyA; }

	SceneNodePtr getBodyB(void)
	{ return _bodyB; }

	/// @internal
	/// @brief progresses the constraint if it's used as an actuator
	/// @param t time since last call, i.e. simulation time step
	virtual void _proggress(vl::time const &t) = 0;

	friend std::ostream &operator<<(std::ostream &, vl::Constraint const &c);

	/// @internal
	void _setLink(vl::animation::LinkRefPtr link);

	/// @internal
	vl::animation::LinkRefPtr _getLink(void) const
	{ return _link; }

protected :
	/// only child classes are allowed to use the constructor
	Constraint(SceneNodePtr rbA, SceneNodePtr rbB, vl::Transform const &worldFrame);

	SceneNodePtr _bodyA;
	SceneNodePtr _bodyB;

	/// Current frames in object coordinates
	vl::Transform _current_local_frame_a;
	vl::Transform _current_local_frame_b;

	vl::animation::LinkRefPtr _link;

};	// class Constraint

class FixedConstraint : public Constraint
{
public :

	/// @internal
	void _proggress(vl::time const &t);

	static FixedConstraintRefPtr create(SceneNodePtr rbA, SceneNodePtr rbB, 
		Transform const &worldFrame)
	{
		FixedConstraintRefPtr constraint(new FixedConstraint(rbA, rbB, worldFrame));
		return constraint;
	}

private :
	FixedConstraint(SceneNodePtr rbA, SceneNodePtr rbB, Transform const &worldFrame);

};	// class FixedConstraint

/* For now no SixDof constraint
class SixDofConstraint : public Constraint
{
public :
	void enableMotor(bool enable);

	bool getMotorEnabled(void) const;

	void setLinearLowerLimit(Ogre::Vector3 const &linearLower);
	
	Ogre::Vector3 const &getLinearLowerLimit(void) const;

	void setLinearUpperLimit(Ogre::Vector3 const &linearUpper);

	Ogre::Vector3 const &getLinearUpperLimit(void) const;

	void setAngularLowerLimit(Ogre::Vector3 const &angularLower);

	Ogre::Vector3 const &getAngularLowerLimit(void) const;

	void setAngularUpperLimit(Ogre::Vector3 const &angularUpper);

	Ogre::Vector3 const &getAngularUpperLimit(void) const;

	static SixDofConstraintRefPtr create(SceneNodePtr rbA, SceneNodePtr rbB, 
		Transform const &worldFrame)
	{
		SixDofConstraintRefPtr constraint(new SixDofConstraint(rbA, rbB, worldFrame));
		return constraint;
	}

	/// @internal
	void _proggress(vl::time const &t);

private :
	SixDofConstraint(SceneNodePtr rbA, SceneNodePtr rbB, Transform const &worldFrame);

	bool _motor_enabled;

	Ogre::Vector3 _linear_lower_limit;
	Ogre::Vector3 _linear_upper_limit;
	Ogre::Vector3 _ang_lower_limit;
	Ogre::Vector3 _ang_upper_limit;

};	// class SixDofConstraint
*/

/** @class SliderConstraint
 *	@brief constraint that allows for one axis of freedom between two bodies
 *	Does not implement dynamics calculation.
 *	Angular limits for slider constraint are not supported
 *	Supports servo motors so that the constraint can be used as an actuator.
 *	@todo rename to prismatic joint, as used in Robot literature
 */
class SliderConstraint : public Constraint
{
public :
	vl::scalar getLowerLimit(void) const;
	
	void setLowerLimit(vl::scalar lowerLimit);

	vl::scalar getUpperLimit(void) const;
	
	void setUpperLimit(vl::scalar upperLimit);

	// Motor
	void enableMotor(bool enable);

	bool getMotorEnabled(void) const;

	/// @todo should we remove the target velocity and replace it by
	/// constant velocity and target position

	/// @brief add to translation target
	void addTarget(vl::scalar target_pos_addition);

	/// @brief set the translation target
	void setMotorTarget(vl::scalar target_pos);

	vl::scalar getMotorTarget(void);

	/// @brief add to target velocity
	void addMotorVelocity(vl::scalar velocity);

	void setMotorVelocity(vl::scalar velocity);

	vl::scalar getMotorVelocity(void) const;

	/// @internal
	void _proggress(vl::time const &t);

	static SliderConstraintRefPtr create(SceneNodePtr rbA, SceneNodePtr rbB, 
		Transform const &worldFrame)
	{
		SliderConstraintRefPtr constraint(new SliderConstraint(rbA, rbB, worldFrame));
		return constraint;
	}

private :
	SliderConstraint(SceneNodePtr rbA, SceneNodePtr rbB, Transform const &worldFrame);

	vl::scalar _lower_limit;
	vl::scalar _upper_limit;
	Ogre::Vector3 _axisInA;

	bool _motor_enabled;
	vl::scalar _target_position;
	vl::scalar _velocity;

};	// class SliderConstraint

/** @class HingeConstraint
 *	@brief a constraint that has all axes locked except for one rotation axis
 *	Supports servo motors for using the constraint as an actuator.
 *	@todo rename to revolute joint, as used in Robot literature
 */
class HingeConstraint : public Constraint
{
public :
	/// @brief change between constraint and actuator
	/// @param enableMotor weather the constraint is an actuator or not
	void enableMotor(bool enableMotor);

	bool getMotorEnabled(void) const;

	/// @brief set the target angle to motor
	/// @param angle target angle which the actuator tries to achieve over time
	void setMotorTarget(Ogre::Radian const &angle);
	
	/// @brief set the velocity of the actuator
	/// @param dt the derivative or how much to change for one time step.
	void setMotorVelocity(Ogre::Radian const &dt);
	
	Ogre::Radian const &getMotorTarget(void) const;
	Ogre::Radian const &getMotorDerivative(void) const;

	void setLowerLimit(Ogre::Radian const &lower);

	Ogre::Radian const &getLowerLimit(void) const;

	void setUpperLimit(Ogre::Radian const &upper);

	Ogre::Radian const &getUpperLimit(void) const;

	// Do we need some extra damping, softness parameters?

	void setAxis(Ogre::Vector3 const &axisInA);

	/// @todo do we store the axis in world or in local, so which one of these is a reference?
	Ogre::Vector3 const &getAxisInA(void) const;

	Ogre::Vector3 getAxisInWorld(void) const;

	Ogre::Radian const &getHingeAngle(void) const;

	/// @internal
	void _proggress(vl::time const &t);

	static HingeConstraintRefPtr create(SceneNodePtr rbA, SceneNodePtr rbB, 
		Transform const &worldFrame)

	{
		HingeConstraintRefPtr constraint(new HingeConstraint(rbA, rbB, worldFrame));
		return constraint;
	}

private :
	HingeConstraint(SceneNodePtr rbA, SceneNodePtr rbB, Transform const &worldFrame);

	Ogre::Radian _lower_limit;
	Ogre::Radian _upper_limit;
	Ogre::Vector3 _axisInA;

	Ogre::Radian _angle;

	bool _motor_enabled;
	Ogre::Radian _target;
	Ogre::Radian _velocity;

};	// class HingeConstraint

}	// namespace vl

#endif	// HYDRA_CONSTRAINTS_HPP
