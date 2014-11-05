/**
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-06
 *	@file animation/constraints.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

/*
 *	Defines basic non-physical constraints. These are based on the common
 *	constraints in physics engines, but do not do the dynamics simulation.
 */

#ifndef HYDRA_CONSTRAINTS_HPP
#define HYDRA_CONSTRAINTS_HPP

#include "typedefs.hpp"

#include "math/math.hpp"
#include "math/transform.hpp"

#include "base/time.hpp"

#include "animation.hpp"

#include <boost/signal.hpp>
// Necessary for generating random names
#include "base/string_utils.hpp"

namespace vl
{

/** @class Constraint
 *	Abstract base class for all kinematic constraints
 */
class HYDRA_API Constraint
{
	typedef boost::signal<void (void)> ChangedCB;

public :
	virtual ~Constraint(void);

	KinematicBodyRefPtr getBodyA(void) const
	{ return _bodyA; }

	KinematicBodyRefPtr getBodyB(void) const
	{ return _bodyB; }

	Transform const &getLocalFrameA(void) const
	{ return _local_frame_a; }

	Transform const &getLocalFrameB(void) const
	{ return _local_frame_b; }

	/// @brief change between constraint and actuator
	/// @param enable weather the constraint is an actuator or not
	virtual void setActuator(bool enable) = 0;

	virtual bool isActuator(void) const = 0;

	virtual vl::scalar getVelocity(void) const = 0;

	virtual void setVelocity(vl::scalar velocity) = 0;

	virtual void addVelocity(vl::scalar velocity) = 0;

	friend std::ostream &operator<<(std::ostream &, vl::Constraint const &c);

	virtual std::string getTypeName(void) const = 0;

	/// @brief get the name of the constraint
	/// If no name has been set this will return empty string
	/// Names are not required to be unique, but if they are not their use
	/// from python interface is really confusing.
	std::string const &getName(void) const
	{ return _name; }

	/// @brief set the name for identifying the constraint
	/// @param name optional name for the constraint
	void setName(std::string const &name)
	{ _name = name; }

	void reset( KinematicBodyRefPtr rbA, KinematicBodyRefPtr rbB,
		vl::Transform const &frameInA, vl::Transform const &frameInB );

	bool isDynamic(void) const
	{ return _is_dynamic; }

	int addListener(ChangedCB::slot_type const &slot)
	{ _changed_cb.connect(slot); return 1; }

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

protected :
	/// @brief Constructor
	/// only child classes are allowed to use the constructor
	/// @param name unique identifier
	/// @param rbA the body to which we want to constraint
	/// @param rbB the body we are constraining
	/// @param frameInA the pivot point in rbA coordinates
	/// @param frameInB the pivot point in rbB coordinates
	Constraint(std::string const &name, KinematicBodyRefPtr rbA, KinematicBodyRefPtr rbB,
		vl::Transform const &frameInA, vl::Transform const &frameInB, bool dynamic);

	std::string _name;

	KinematicBodyRefPtr _bodyA;
	KinematicBodyRefPtr _bodyB;

	/// Current frames in object coordinates
	vl::Transform _local_frame_a;
	vl::Transform _local_frame_b;

	vl::animation::LinkRefPtr _link;

	bool _is_dynamic;

	ChangedCB _changed_cb;

};	// class Constraint

class HYDRA_API FixedConstraint : public Constraint
{
public :

	/// Abstract overrides
	virtual void setActuator(bool enable) {}

	virtual bool isActuator(void) const { return false; }

	virtual vl::scalar getVelocity(void) const { return 0; }

	virtual void setVelocity(vl::scalar velocity) {}

	virtual void addVelocity(vl::scalar velocity) {}

	virtual std::string getTypeName(void) const
	{ return "fixed"; }

	// static
	static FixedConstraintRefPtr create(std::string const &name, 
		KinematicBodyRefPtr rbA, KinematicBodyRefPtr rbB,
		 vl::Transform const &frameInA, vl::Transform const &frameInB, bool dynamic)
	{
		FixedConstraintRefPtr constraint(new FixedConstraint(name, rbA, rbB, frameInA, frameInB, dynamic));
		return constraint;
	}

	// @todo we should add a method without frame because the frame is not
	// needed for anything as long as the constraint is fixed.

	/// Private virtual overrides
private :
	/// @internal
	void _progress(vl::time const &t);

	FixedConstraint(std::string const &name, KinematicBodyRefPtr rbA,
		KinematicBodyRefPtr rbB, Transform const &frameInA, 
		Transform const &frameInB, bool dynamic);

};	// class FixedConstraint

/** @class SliderConstraint
 *	@brief constraint that allows for one axis of freedom between two bodies
 *	Does not implement dynamics calculation.
 *	Angular limits for slider constraint are not supported
 *	Supports servo motors so that the constraint can be used as an actuator.
 *	@todo rename to prismatic joint, as used in Robot literature
 */
class HYDRA_API SliderConstraint : public Constraint
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

	virtual vl::scalar getVelocity(void) const
	{ return vl::sign(_target_position)*_speed; }

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

	/// Default axis UNIT_Z
	Ogre::Vector3 const &getAxis(void) const
	{ return _axisInA; }

	void setAxis(Ogre::Vector3 const &v)
	{
		_axisInA = v;
		_axisInA.normalise();
	}

	vl::scalar getPosition(void) const;

	void setPosition(vl::scalar pos);

	virtual std::string getTypeName(void) const
	{ return "slider"; }

	// static
	static SliderConstraintRefPtr create(std::string const &name,
		KinematicBodyRefPtr rbA, KinematicBodyRefPtr rbB,
		Transform const &frameInA, Transform const &frameInB, bool dynamic)
	{
		SliderConstraintRefPtr constraint(new SliderConstraint(name, rbA, rbB, frameInA, frameInB, dynamic));
		return constraint;
	}


	/// Private virtual overrides
private :
	/// @internal
	void _progress(vl::time const &t);

	SliderConstraint(std::string const &name, KinematicBodyRefPtr rbA,
		KinematicBodyRefPtr rbB, Transform const &frameInA,
		Transform const &frameInB, bool dynamic);
	
	vl::scalar _lower_limit;
	vl::scalar _upper_limit;
	Ogre::Vector3 _axisInA;

	bool _actuator;
	vl::scalar _target_position;
	// @fixme having speed (always positive) and switching target position 
	// to positive and negative upper limit is a brain dead design choice.
	vl::scalar _speed;

};	// class SliderConstraint

/** @class HingeConstraint
 *	@brief a constraint that has all axes locked except for one rotation axis
 *	Supports servo motors for using the constraint as an actuator.
 *	@todo rename to revolute joint, as used in Robot literature
 *
 *	@todo If FPS is much greater than 60 the progress method does not work correctly
 *	This is probably because there is inaccuracies in calculating the angle from
 *	quaternions and comparing them.
 */
class HYDRA_API HingeConstraint : public Constraint
{
public :
	/// @brief change between constraint and actuator
	/// @param enable weather the constraint is an actuator or not
	virtual void setActuator(bool enable)
	{ _actuator = enable; }

	virtual bool isActuator(void) const
	{ return _actuator; }

	virtual vl::scalar getVelocity(void) const
	{ return Ogre::Radian(vl::sign(_target)*_speed).valueRadians(); }

	/// Sets the target to maximum and controls the approaching velocity
	/// provides a servo motor control for the constraint
	/// @todo this should be moved to separate motor/actuator class
	virtual void setVelocity(vl::scalar velocity);

	virtual void addVelocity(vl::scalar velocity);

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

	/// Default axis UNIT_Z
	Ogre::Vector3 const &getAxis(void) const
	{ return _axisInA; }

	void setAxis(Ogre::Vector3 const &v)
	{
		_axisInA = v;
		_axisInA.normalise();
	}

	Ogre::Vector3 getAxisInWorld(void) const;

	/// @brief returns the angle the hinge is in along the path moved.
	Ogre::Radian getHingeAngle(void) const;

	void setHingeAngle(Ogre::Radian const &angle);

	virtual std::string getTypeName(void) const
	{ return "hinge"; }

	// static
	static HingeConstraintRefPtr create(std::string const &name, KinematicBodyRefPtr rbA, 
		KinematicBodyRefPtr rbB, Transform const &frameInA, Transform const &frameInB, bool dynamic)
	{
		HingeConstraintRefPtr constraint(new HingeConstraint(name, rbA, rbB, frameInA, frameInB, dynamic));
		return constraint;
	}


	/// Private virtual overrides
private :
	/// @internal
	void _progress(vl::time const &t);

	HingeConstraint(std::string const &name, KinematicBodyRefPtr rbA,
		KinematicBodyRefPtr rbB, Transform const &frameInA,
		Transform const &frameInB, bool dynamic);

	Ogre::Radian _lower_limit;
	Ogre::Radian _upper_limit;
	Ogre::Vector3 _axisInA;

	bool _actuator;
	Ogre::Radian _target;
	// @fixme same bs here speed should be velocity
	Ogre::Radian _speed;

};	// class HingeConstraint

std::ostream &
operator<<(std::ostream &os, HingeConstraint const &c);

std::ostream &
operator<<(std::ostream &os, SliderConstraint const &c);

std::ostream &
operator<<(std::ostream &os, ConstraintList const &list);

}	// namespace vl

#endif	// HYDRA_CONSTRAINTS_HPP
