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

/// Necessary for constraint actions
#include "action.hpp"

namespace vl
{

class Constraint
{
public :
	virtual ~Constraint(void)
	{}

	KinematicBodyRefPtr getBodyA(void) const
	{ return _bodyA; }

	KinematicBodyRefPtr getBodyB(void) const
	{ return _bodyB; }

	/// @brief change between constraint and actuator
	/// @param enable weather the constraint is an actuator or not
	virtual void setActuator(bool enable) = 0;

	virtual bool isActuator(void) const = 0;

	virtual void setVelocity(vl::scalar velocity) = 0;

	virtual void addVelocity(vl::scalar velocity) = 0;

	friend std::ostream &operator<<(std::ostream &, vl::Constraint const &c);

	/// @internal
	void _solve(vl::time const &t);

	/// @internal
	void _setLink(vl::animation::LinkRefPtr link);

	/// @internal
	vl::animation::LinkRefPtr _getLink(void) const
	{ return _link; }

	/// Private virtuals
private :
	/// @internal
	/// @brief progresses the constraint if it's used as an actuator
	/// @param t time since last call, i.e. simulation time step
	virtual void _progress(vl::time const &t) = 0;

	/// @todo this is not used for anything as it does not work
	/// as excepted.
	/// Left because we don't have any other implementations planned
	/// in the near future so this might be useful when such implementation
	/// is going to be done.
	void _solve_aux_parents(void);

protected :
	/// only child classes are allowed to use the constructor
	Constraint(KinematicBodyRefPtr rbA, KinematicBodyRefPtr rbB, vl::Transform const &worldFrame);

	KinematicBodyRefPtr _bodyA;
	KinematicBodyRefPtr _bodyB;

	/// Current frames in object coordinates
	vl::Transform _local_frame_a;
	vl::Transform _local_frame_b;

	vl::animation::LinkRefPtr _link;

};	// class Constraint

class FixedConstraint : public Constraint
{
public :

	/// Abstract overrides
	virtual void setActuator(bool enable) {}

	virtual bool isActuator(void) const { return false; }

	virtual void setVelocity(vl::scalar velocity) {}

	virtual void addVelocity(vl::scalar velocity) {}

	static FixedConstraintRefPtr create(KinematicBodyRefPtr rbA, KinematicBodyRefPtr rbB, 
		Transform const &worldFrame)
	{
		FixedConstraintRefPtr constraint(new FixedConstraint(rbA, rbB, worldFrame));
		return constraint;
	}

	/// Private virtual overrides
private :
	/// @internal
	void _progress(vl::time const &t);

private :
	FixedConstraint(KinematicBodyRefPtr rbA, KinematicBodyRefPtr rbB, Transform const &worldFrame);

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
	vl::scalar getLowerLimit(void) const
	{ return _lower_limit; }
	
	void setLowerLimit(vl::scalar lowerLimit);

	vl::scalar getUpperLimit(void) const
	{ return _upper_limit; }
	
	void setUpperLimit(vl::scalar upperLimit);

	/// @brief change between constraint and actuator
	/// @param enable weather the constraint is an actuator or not
	virtual void setActuator(bool enable)
	{ _actuator = enable; }

	virtual bool isActuator(void) const
	{ return _actuator; }

	/// Sets the target to maximum and controls the approaching velocity
	/// provides a servo motor control for the constraint
	virtual void setVelocity(vl::scalar velocity);

	virtual void addVelocity(vl::scalar velocity);

	/// @todo should we remove the target velocity and replace it by
	/// constant velocity and target position

	/// @brief add to translation target
	void addActuatorTarget(vl::scalar target_pos_addition);

	/// @brief set the translation target
	void setActuatorTarget(vl::scalar target_pos);

	vl::scalar getActuatorTarget(void) const
	{ return _target_position; }

	/// @brief add to target speed, the final result is clamped positive
	/// @param velocity to add to speed can be either negative or positive
	void addActuatorSpeed(vl::scalar velocity);

	/// @brief set the motor speed, the speed is always positive
	/// @param velocity, the velocity of the motor negative values are clamped to zero
	void setActuatorSpeed(vl::scalar velocity);

	vl::scalar getActuatorSpeed(void) const
	{ return _speed; }

	Ogre::Vector3 const &getAxis(void) const
	{ return _axisInA; }

	void setAxis(Ogre::Vector3 const &v)
	{ _axisInA = v; }

	vl::scalar getPosition(void) const;

	static SliderConstraintRefPtr create(KinematicBodyRefPtr rbA, KinematicBodyRefPtr rbB, 
		Transform const &worldFrame)
	{
		SliderConstraintRefPtr constraint(new SliderConstraint(rbA, rbB, worldFrame));
		return constraint;
	}

	/// Private virtual overrides
private :
	/// @internal
	void _progress(vl::time const &t);

private :
	SliderConstraint(KinematicBodyRefPtr rbA, KinematicBodyRefPtr rbB, Transform const &worldFrame);

	vl::scalar _lower_limit;
	vl::scalar _upper_limit;
	Ogre::Vector3 _axisInA;

	bool _actuator;
	vl::scalar _target_position;
	vl::scalar _speed;

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
	/// @param enable weather the constraint is an actuator or not
	virtual void setActuator(bool enable)
	{ _actuator = enable; }

	virtual bool isActuator(void) const
	{ return _actuator; }

	/// Sets the target to maximum and controls the approaching velocity
	/// provides a servo motor control for the constraint
	virtual void setVelocity(vl::scalar velocity);

	virtual void addVelocity(vl::scalar velocity);

	void addActuatorTarget(Ogre::Radian const &angle)
	{ setActuatorTarget(angle+_angle); }

	/// @brief set the target angle to motor
	/// @param angle target angle which the actuator tries to achieve over time
	void setActuatorTarget(Ogre::Radian const &angle);

	Ogre::Radian const &getActuatorTarget(void) const
	{ return _target; }

	/// @brief set the velocity of the actuator
	/// @param dt the derivative or how much to change for one time step.
	void setActuatorSpeed(Ogre::Radian const &dt);

	Ogre::Radian const &getActuatorSpeed(void) const
	{ return _speed; }

	void setLowerLimit(Ogre::Radian const &lower);

	Ogre::Radian const &getLowerLimit(void) const
	{ return _lower_limit; }

	void setUpperLimit(Ogre::Radian const &upper);

	Ogre::Radian const &getUpperLimit(void) const
	{ return _upper_limit; }

	// Do we need some extra damping, softness parameters?

	Ogre::Vector3 const &getAxis(void) const
	{ return _axisInA; }

	void setAxis(Ogre::Vector3 const &v)
	{ _axisInA = v; }

	Ogre::Vector3 getAxisInWorld(void) const;

	Ogre::Radian const &getHingeAngle(void) const
	{ return _angle; }

	static HingeConstraintRefPtr create(KinematicBodyRefPtr rbA, KinematicBodyRefPtr rbB, 
		Transform const &worldFrame)
	{
		HingeConstraintRefPtr constraint(new HingeConstraint(rbA, rbB, worldFrame));
		return constraint;
	}

	/// Private virtual overrides
private :
	/// @internal
	void _progress(vl::time const &t);

private :
	HingeConstraint(KinematicBodyRefPtr rbA, KinematicBodyRefPtr rbB, Transform const &worldFrame);

	Ogre::Radian _lower_limit;
	Ogre::Radian _upper_limit;
	Ogre::Vector3 _axisInA;

	Ogre::Radian _angle;

	bool _actuator;
	Ogre::Radian _target;
	Ogre::Radian _speed;

};	// class HingeConstraint

std::ostream &
operator<<(std::ostream &os, HingeConstraint const &c);

std::ostream &
operator<<(std::ostream &os, SliderConstraint const &c);

class SliderActuatorAction : public vl::BasicAction
{
public :
	SliderActuatorAction(void)
		: target(0)
	{}

	virtual void execute(void)
	{
		if(constraint)
		{ constraint->addActuatorTarget(target); }
	}

	virtual std::string getTypeName( void ) const
	{ return "SliderActuatorAction"; }

	static SliderActuatorAction *create(void)
	{ return new SliderActuatorAction; }

	vl::scalar target;

	vl::SliderConstraintRefPtr constraint;

};

class HingeActuatorAction : public vl::BasicAction
{
public :
	HingeActuatorAction(void)
		: target(0)
	{}

	virtual void execute(void)
	{
		if(constraint)
		{ constraint->addActuatorTarget(target); }
	}

	virtual std::string getTypeName( void ) const
	{ return "HingeActuatorAction"; }

	static HingeActuatorAction *create(void)
	{ return new HingeActuatorAction; }

	Ogre::Radian target;

	vl::HingeConstraintRefPtr constraint;

};


}	// namespace vl

#endif	// HYDRA_CONSTRAINTS_HPP
