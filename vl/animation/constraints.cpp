/**
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-06
 *	@file animation/constraints.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

/*
 *	Defines basic non-physical constraints. These are based on the common
 *	constraints in physics engines, but do not do the dynamics simulation.
 */

#include "constraints.hpp"

#include "scene_node.hpp"

#include "base/exceptions.hpp"

#include "kinematic_body.hpp"

#include "logger.hpp"

std::ostream &
vl::operator<<(std::ostream &os, vl::Constraint const &c)
{
	os << "Constraint with bodies : " << c._bodyA->getName() << " and " 
		<< c._bodyB->getName() << "\n"
		<< "   Transformation = " << c._getLink()->getTransform() << "\n"
		<< "   Initial transformation = " << c._getLink()->getInitialTransform()
		<< std::endl;

	return os;
}

std::ostream &
vl::operator<<(std::ostream &os, vl::HingeConstraint const &c)
{
	if(c.getName().empty())
	{
		os << "Unamed HingeConstraint" << "\n";
	}
	else
	{
		os << "HingeConstraint with name " << c.getName() << "\n";
	}
	os << "with bodies : " << c.getBodyA()->getName() << " and " 
		<< c.getBodyB()->getName() << "\n"
		<< "   Transformation = " << c._getLink()->getTransform() << "\n"
		<< "   Initial transformation = " << c._getLink()->getInitialTransform() << "\n";
	if(c.isActuator())
	{
		os << "   target : = " << c.getActuatorTarget() << " lower limit = " 
			<< c.getLowerLimit() << " upper limit = " << c.getUpperLimit()
			<< " speed = " << c.getActuatorSpeed();
	}
	else
	{ os << "   Not an actuator."; }
	os << std::endl;

	return os;
}

std::ostream &
vl::operator<<(std::ostream &os, vl::SliderConstraint const &c)
{
	if(c.getName().empty())
	{
		os << "Unamed SliderConstraint " << "\n";
	}
	else
	{
		os << "SliderConstraint with name " << c.getName() << "\n";
	}
	os << " with bodies : " << c.getBodyA()->getName() << " and " 
		<< c.getBodyB()->getName() << "\n"
		<< "   Transformation = " << c._getLink()->getTransform() << "\n"
		<< "   Initial transformation = " << c._getLink()->getInitialTransform() << "\n";
	if(c.isActuator())
	{
		os << "   target : = " << c.getActuatorTarget() << " lower limit = " 
			<< c.getLowerLimit() << " upper limit = " << c.getUpperLimit()
			<< " speed = " << c.getActuatorSpeed();
	}
	else
	{ os << "   Not an actuator."; }
	os << std::endl;

	return os;
}

std::ostream &
vl::operator<<(std::ostream &os, ConstraintList const &list)
{
	for(ConstraintList::const_iterator iter = list.begin();
		iter != list.end(); ++iter)
	{
		if((*iter)->getName().empty())
		{
			os << "Unnamed " << (*iter)->getTypeName() << " constraint." << std::endl;
		}
		else
		{
			os << (*iter)->getTypeName() << " constraint with name " << (*iter)->getName() << std::endl;
		}
	}

	return os;
}


/// ------------------------------ Constraint --------------------------------
/// ------------------------------ Public ------------------------------------
vl::Constraint::~Constraint(void)
{
	if(_link)
	{
		_link->setParent(animation::NodeRefPtr());
		_link->setChild(animation::NodeRefPtr());
	}
}

void
vl::Constraint::reset( KinematicBodyRefPtr rbA, KinematicBodyRefPtr rbB,
	vl::Transform const &frameInA, vl::Transform const &frameInB )
{
	assert(rbA && rbB);

	_bodyA = rbA;
	_bodyB = rbB;
	_local_frame_a = frameInA;
	_local_frame_b = frameInB;

	assert(_link);

	_link->setTransform(_local_frame_a, true);
	_link->setInitialState();
	_local_frame_b = _link->getChild()->getTransform();
}

/// ------------------------------ Protected ---------------------------------
vl::Constraint::Constraint(std::string const &name, KinematicBodyRefPtr rbA, 
		KinematicBodyRefPtr rbB, vl::Transform const &frameInA, 
		vl::Transform const &frameInB, bool dynamic)
	: _name(name)
	, _bodyA(rbA)
	, _bodyB(rbB)
	, _local_frame_a(frameInA)
	, _local_frame_b(frameInB)
	, _is_dynamic(dynamic)
{
	if(!_bodyA || !_bodyB)
	{
		std::string err_msg("Missing a body.");
		std::cout << vl::CRITICAL << err_msg << std::endl;
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(err_msg));
	}
}

void
vl::Constraint::_setLink(vl::animation::LinkRefPtr link)
{
	if(!link)
	{
		std::string err_msg("Trying to set NULL link.");
		std::cout << vl::CRITICAL << err_msg << std::endl;
		BOOST_THROW_EXCEPTION(vl::null_pointer() << vl::desc(err_msg));
	}
	if(_link)
	{
		std::string err_msg("Resetting a Link is not supported.");
		std::cout << vl::CRITICAL << err_msg << std::endl;
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(err_msg)); 
	}

	_link = link;

	_link->setTransform(_local_frame_a, true);
	_link->setInitialState();
	_local_frame_b = _link->getChild()->getTransform();
}

void
vl::Constraint::_solve(vl::time const &t)
{
	_progress(t);
}

/// ------------------------------ FixedConstraint ---------------------------
/// ------------------------------ Public ------------------------------------
void
vl::FixedConstraint::_progress(vl::time const &t)
{
	// Purposefully empty the animation engine will handle following of an
	// another object be that a real parent or an auxilary parent.
	// progress should only update link information which for FixedConstraint
	// is always fixed.
}

/// ------------------------------ Private -----------------------------------
vl::FixedConstraint::FixedConstraint(std::string const &name, 
		KinematicBodyRefPtr rbA, KinematicBodyRefPtr rbB, 
		Transform const &frameInA, Transform const &frameInB, bool dynamic)
	: Constraint(name, rbA, rbB, frameInA, frameInB, dynamic)
{}

/// ------------------------------ SliderConstraint --------------------------
/// ------------------------------ Public ------------------------------------	
void
vl::SliderConstraint::setVelocity(vl::scalar velocity)
{
	// @todo does this need to set to lower limit if velocity is negative?
	// yes because speed is always positive
	if(velocity < 0)
	{ _target_position = _lower_limit; }
	else
	{ _target_position = _upper_limit; }
	_speed = vl::abs(velocity);

	_changed_cb();
}

void
vl::SliderConstraint::addVelocity(vl::scalar velocity)
{
	vl::scalar sign = vl::sign(_target_position);
	setVelocity(velocity + sign*_speed);
}

void
vl::SliderConstraint::setLowerLimit(vl::scalar lowerLimit)
{
	_lower_limit = lowerLimit;
	_changed_cb();
}

void
vl::SliderConstraint::setUpperLimit(vl::scalar upperLimit)
{
	_upper_limit = upperLimit;
	_changed_cb();
}

void 
vl::SliderConstraint::addActuatorTarget(vl::scalar target_pos_addition)
{
	_target_position += target_pos_addition;
	_changed_cb();
}

void 
vl::SliderConstraint::setActuatorTarget(vl::scalar target_pos)
{
	_target_position = target_pos;
	_changed_cb();
}

void
vl::SliderConstraint::addActuatorSpeed(vl::scalar velocity)
{
	_speed = vl::max(vl::scalar(0), _speed + velocity);
	_changed_cb();
}

void
vl::SliderConstraint::setActuatorSpeed(vl::scalar velocity)
{
	_speed = vl::max(vl::scalar(0), velocity);
	_changed_cb();
}

vl::scalar
vl::SliderConstraint::getPosition(void) const
{
	if(!_link)
	{ return 0; }

	return _link->getTransform().position.y - _link->getInitialTransform().position.y;
}

void
vl::SliderConstraint::setPosition(vl::scalar pos)
{
	if(!_link)
	{ return; }

	_link->setPosition(_link->getInitialTransform().position + pos*_axisInA);
}

void
vl::SliderConstraint::_progress(vl::time const &t)
{
	if(!_link)
	{ return; }

	Ogre::Vector3 v = _link->getTransform().position - _link->getInitialTransform().position;
	vl::scalar pos_size = _axisInA.dotProduct(v);
	vl::scalar mov = 0;

	if(isActuator() && _target_position != pos_size)
	{
		/// Axis is either not limited (lower limit is greater than upper)
		/// or if it's current position is within the limits
		/// @todo should we update the target position based on limits, 
		/// so that there is no impossible target

		vl::scalar s = vl::sign(_target_position - pos_size);
		vl::scalar m_step = vl::abs(_speed) * double(t);
		vl::scalar max_step = vl::abs(_target_position - pos_size); 
		mov = s * std::min(m_step, max_step); 
	}
	else
	{
		// @todo this should handle free movement following the A object
	}
	
	/// Move the child body
	/// Wether the joint is limited or not
	/// @todo this is common for all joints, though the constraint type linear/angular changes
	bool free = _lower_limit > _upper_limit;
	// Clamp only if not free
	if(!free)
	{
		vl::clamp(mov, _lower_limit-pos_size, _upper_limit-pos_size);
	}

	if(mov != 0)
	{
		/// Update object B
		/// needs to call setTransform because it does other things besides
		/// updating the transformation.
		_link->setPosition(_link->getTransform().position + mov*_axisInA);
	}
}

/// ------------------------------ Private -----------------------------------
vl::SliderConstraint::SliderConstraint(std::string const &name, 
		KinematicBodyRefPtr rbA, KinematicBodyRefPtr rbB, 
		Transform const &frameInA, Transform const &frameInB, bool dynamic)
	: Constraint(name, rbA, rbB, frameInA, frameInB, dynamic)
	, _lower_limit(0)
	, _upper_limit(0)
	, _axisInA(0, 0, 1)
	, _actuator(false)
	, _target_position(0)
	, _speed(1)
{}

/// ------------------------------ HingeConstraint ---------------------------
/// ------------------------------ Public ------------------------------------	
void
vl::HingeConstraint::setVelocity(vl::scalar velocity)
{
	// @todo does this need to set to lower limit if velocity is negative?
	// yes because speed is always positive
	if(velocity < 0)
	{ _target = _lower_limit; }
	else
	{ _target = _upper_limit; }
	_speed = Ogre::Radian(vl::abs(velocity));
	_changed_cb();
}

void
vl::HingeConstraint::addVelocity(vl::scalar velocity)
{
	vl::scalar sign = vl::sign(_target.valueRadians());
	setVelocity(sign*_speed.valueRadians() + velocity);
}

void
vl::HingeConstraint::setActuatorTarget(Ogre::Radian const &angle)
{
	_target = angle;
	_changed_cb();
}

void 
vl::HingeConstraint::setActuatorSpeed(Ogre::Radian const &dt)
{
	_speed = vl::max(Ogre::Radian(0), dt);
	_changed_cb();
}

void
vl::HingeConstraint::setLowerLimit(Ogre::Radian const &lower)
{
	_lower_limit = lower;
	_changed_cb();
}

void
vl::HingeConstraint::setUpperLimit(Ogre::Radian const &upper)
{
	_upper_limit = upper;
	_changed_cb();
}

Ogre::Vector3
vl::HingeConstraint::getAxisInWorld(void) const
{
	// @todo should probably use the link transformation
	return _bodyA->getWorldTransform()*_axisInA;
}

Ogre::Radian
vl::HingeConstraint::getHingeAngle(void) const
{
	if(!_link)
	{ return Ogre::Radian(0); }

	Ogre::Quaternion const &current_q = _link->getTransform().quaternion;
	Ogre::Quaternion const &init_q =_link->getInitialTransform().quaternion;

	Ogre::Quaternion q = init_q.Inverse()*current_q;
	q.normalise();
	Ogre::Radian angle;
	Ogre::Vector3 axis;
	q.ToAngleAxis(angle, axis);

	/// @todo add checking for negative
	if( !vl::equal(angle, Ogre::Radian(0), Ogre::Radian(EPSILON)) )
	{
		// reverse the angle if we have a negative axis
		if( vl::equal(-axis, _axisInA) )
		{
			// @todo is this correct ?
			angle = -angle;
		}
		else if( !vl::equal(axis, _axisInA) )
		{
			//std::clog << "vl::HingeConstraint::getHingeAngle : axis of rotation is incorrect \n"
			//	<< "\trotation axis = " << _axisInA << " got " << axis << " angle = " << angle << std::endl;
		}
	}

	return angle;
}

void
vl::HingeConstraint::setHingeAngle(Ogre::Radian const &angle)
{
	if(!_link)
	{ return; }

	Ogre::Quaternion q(angle, _axisInA);			
	_link->setOrientation(_link->getInitialTransform().quaternion*q);
}

void
vl::HingeConstraint::_progress(vl::time const &t)
{
	// No state variables should be stored in the Constraint itself
	// because the Constraint is not updated from collision detection
	// the Link and Node in the animation namespace are so
	// we should use their state (transformation) for calculating
	// all temporary values we need here.

	if(!_link)
	{ return; }

	// for now following constraints are not supported
	if(!isActuator() || _speed == Ogre::Radian(0))
	{ return; }

	// Maximum allowed change per timestep
	Ogre::Radian max_angle = _speed*t;

	if(vl::equal(max_angle, Ogre::Radian(0), Ogre::Radian(EPSILON)))
	{ return; }

	Ogre::Radian current_angle = getHingeAngle();
	// The difference from current position to target
	Ogre::Radian to_target = _target - current_angle;
	// The angle derivative
	Ogre::Radian angle_d = vl::sign(to_target) * vl::min(max_angle, vl::abs(to_target));

	// Constraint is free if upper limit is less than lower limit
	if( _upper_limit >= _lower_limit )
	{	
		Ogre::Radian new_angle = current_angle + angle_d;
		vl::clamp(new_angle, _lower_limit, _upper_limit);
		assert( angle_d > new_angle - current_angle || 
			vl::equal(angle_d, new_angle - current_angle, Ogre::Radian(EPSILON)) );
		angle_d = new_angle - current_angle;
	}

	// Only one point where the transformation occurs this ensures that
	// there will never be more than one transformation
	// also check for small errors so we don't get vibrations
	if( !vl::equal(angle_d.valueRadians(), vl::scalar(0.0), vl::scalar(1e-3)) )
	{
		Ogre::Quaternion q(angle_d, _axisInA);			
		_link->rotate(q);
	}
}

/// ------------------------------ Private -----------------------------------
vl::HingeConstraint::HingeConstraint(std::string const &name,
	KinematicBodyRefPtr rbA, KinematicBodyRefPtr rbB,
	Transform const &frameInA, Transform const &frameInB, bool dynamic)
	: Constraint(name, rbA, rbB, frameInA, frameInB, dynamic)
	, _lower_limit()
	, _upper_limit()
	, _axisInA(0, 0, 1)
	, _actuator(false)
	, _target()
	, _speed(Ogre::Degree(30))
{}
