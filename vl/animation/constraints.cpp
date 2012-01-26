/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-06
 *	@file animation/constraints.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
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
	os << "HingeConstraint with bodies : " << c.getBodyA()->getName() << " and " 
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
	os << "SliderConstraint with bodies : " << c.getBodyA()->getName() << " and " 
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

/// ------------------------------ Constraint --------------------------------
/// ------------------------------ Public ------------------------------------

/// ------------------------------ Protected ---------------------------------
vl::Constraint::Constraint(KinematicBodyRefPtr rbA, KinematicBodyRefPtr rbB, vl::Transform const &worldFrame)
	: _bodyA(rbA)
	, _bodyB(rbB)
{
	if(!_bodyA || !_bodyB)
	{
		std::string err_msg("Missing a body.");
		std::cout << vl::CRITICAL << err_msg << std::endl;
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(err_msg));
	}

	vl::Transform wtA(_bodyA->getWorldTransform());
	wtA.invert();

	_local_frame_a = wtA*worldFrame;
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

void
vl::Constraint::_solve_aux_parents(void)
{
	assert(_link);

	std::vector<animation::LinkRefPtr> aux_parents = _link->getChild()->getAuxilaryParents();
	if(aux_parents.empty())
	{ return; }

	for(std::vector<animation::LinkRefPtr>::iterator p_iter = aux_parents.begin();
		p_iter != aux_parents.end(); ++p_iter)
	{
		// Just testing something
		// p_iter parent is the auxilary parent that should follow the child
		// parent is the link that has been modified.
		// @todo initial transformation for the p_iter parent 
		// needs be set and we need to calculate
		// this using a transformation to local space.
		animation::NodeRefPtr node = (*p_iter)->getParent();
		animation::LinkRefPtr ref_link = node->getParent();
		// This needs to be the ref link instead of Node because otherwise
		// we are setting the same transformation over and over.
		// Because we are modifying the transformation of "node"
		// so we need to use it's parent for the world transformation.
		Transform wtA = ref_link->getWorldTransform();
		wtA.invert();
		// Initial transfromation is in the Link
		// The initial transformation is correct, because without it
		// the object is offsetted to the right (same if not inverted).
		// diffA is the difference between node and the child link.
		Transform diffA = (*p_iter)->getTransform();
		diffA.invert();
		//Transform link_world = (*p_iter)->getWorldTransform();
		// transform real parent transformation to local space and then apply
		// the initial transformation from the link to it.
		// @todo fix the transformation
		// should we get the child or link world transformation?
		// child because we want to follow the child not the link.
		// we need to transform the node, because transforming the link would have
		// no effect (it only transforms the child).
		Transform t(Quaternion(0, 0, 0, 1));
		Transform t2(Quaternion(0.7071, 0, 0, 0.7071));
		node->setTransform(wtA * _link->getChild()->getWorldTransform() * t2*diffA);
		// @todo this needs to go through the whole kinematic chain to the base
		// and update all nodes that are linked as a parent to the modified link.
	}
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
vl::FixedConstraint::FixedConstraint(KinematicBodyRefPtr rbA, KinematicBodyRefPtr rbB, Transform const &worldFrame)
	: Constraint(rbA, rbB, worldFrame)
{}

/// ------------------------------ SixDofConstraint --------------------------
/*
/// ------------------------------ Public ------------------------------------	
void
vl::SixDofConstraint::enableMotor(bool enable)
{
	_motor_enabled = enable;
}

bool
vl::SixDofConstraint::getMotorEnabled(void) const
{
	return _motor_enabled;
}

void
vl::SixDofConstraint::setLinearLowerLimit(Ogre::Vector3 const &linearLower)
{
	_linear_lower_limit = linearLower;
}

Ogre::Vector3 const &
vl::SixDofConstraint::getLinearLowerLimit(void) const
{
	return _linear_lower_limit;
}

void
vl::SixDofConstraint::setLinearUpperLimit(Ogre::Vector3 const &linearUpper)
{
	_linear_upper_limit = linearUpper;
}

Ogre::Vector3 const &
vl::SixDofConstraint::getLinearUpperLimit(void) const
{
	return _linear_upper_limit;
}

void
vl::SixDofConstraint::setAngularLowerLimit(Ogre::Vector3 const &angularLower)
{
	_ang_lower_limit = angularLower;
}

Ogre::Vector3 const &
vl::SixDofConstraint::getAngularLowerLimit(void) const
{
	return _ang_lower_limit;
}

void
vl::SixDofConstraint::setAngularUpperLimit(Ogre::Vector3 const &angularUpper)
{
	_ang_upper_limit = angularUpper;
}

Ogre::Vector3 const &
vl::SixDofConstraint::getAngularUpperLimit(void) const
{
	return _ang_upper_limit;
}

void
vl::SixDofConstraint::_proggress(vl::time const &t)
{
	std::vector<bool> free;
	free.push_back(_linear_upper_limit.x < _linear_lower_limit.x);
	free.push_back(_linear_upper_limit.y < _linear_lower_limit.y);
	free.push_back(_linear_upper_limit.z < _linear_lower_limit.z);
	free.push_back(_ang_upper_limit.z < _ang_lower_limit.z);
	free.push_back(_ang_upper_limit.z < _ang_lower_limit.z);
	free.push_back(_ang_upper_limit.z < _ang_lower_limit.z);

	if(_motor_enabled)
	{
	}
	else
	{
	}

	_update_bodies();
}

/// ------------------------------ Private -----------------------------------
vl::SixDofConstraint::SixDofConstraint(SceneNodePtr rbA, SceneNodePtr rbB, Transform const &worldFrame)
	: Constraint(rbA, rbB, worldFrame)
	, _motor_enabled(false)
	, _linear_lower_limit(0, 0, 0)
	, _linear_upper_limit(0, 0, 0)
	, _ang_lower_limit(0, 0, 0)
	, _ang_upper_limit(0, 0, 0)
{}
*/

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
}

void
vl::SliderConstraint::setUpperLimit(vl::scalar upperLimit)
{
	_upper_limit = upperLimit;
}

void 
vl::SliderConstraint::addActuatorTarget(vl::scalar target_pos_addition)
{ _target_position += target_pos_addition; }

void 
vl::SliderConstraint::setActuatorTarget(vl::scalar target_pos)
{
	_target_position = target_pos; 
}

void
vl::SliderConstraint::addActuatorSpeed(vl::scalar velocity)
{
	_speed = vl::max(vl::scalar(0), _speed + velocity);
}

void
vl::SliderConstraint::setActuatorSpeed(vl::scalar velocity)
{
	_speed = vl::max(vl::scalar(0), velocity);
}

vl::scalar
vl::SliderConstraint::getPosition(void) const
{
	return _link->getTransform().position.y - _link->getInitialTransform().position.y;
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
		_link->setTransform(_link->getTransform().position + mov*_axisInA);
	}
}

/// ------------------------------ Private -----------------------------------
vl::SliderConstraint::SliderConstraint(KinematicBodyRefPtr rbA, KinematicBodyRefPtr rbB, Transform const &worldFrame)
	: Constraint(rbA, rbB, worldFrame)
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
}

void 
vl::HingeConstraint::setActuatorSpeed(Ogre::Radian const &dt)
{
	_speed = vl::max(Ogre::Radian(0), dt);
}

void
vl::HingeConstraint::setLowerLimit(Ogre::Radian const &lower)
{
	_lower_limit = lower;
}

void
vl::HingeConstraint::setUpperLimit(Ogre::Radian const &upper)
{
	_upper_limit = upper;
}

Ogre::Vector3
vl::HingeConstraint::getAxisInWorld(void) const
{
	// @todo should probably use the link transformation
	return _bodyA->getWorldTransform()*_axisInA;
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

	// Calculate the progression rate using angular velocity
	Ogre::Radian max_angle = _speed*t;
	Ogre::Radian angle = max_angle;

	Ogre::Quaternion const &current_q(_link->getTransform().quaternion);
	Ogre::Quaternion target_q = _link->getInitialTransform().quaternion*Quaternion(_target, _axisInA) ;

	vl::scalar dist = distance(current_q, target_q);
	// clamped version of the distance and maximum movement per step
	vl::scalar limit = vl::min(angle.valueRadians(), dist);

	// needs some tolerance because of inaccuracies in the quaternions
	// and maybe some missing normalisations.
	vl::scalar epsilon = 0.001;
	if(!vl::equal(dist, vl::scalar(0), epsilon)
		&& !vl::equal(angle, Ogre::Radian(0), Ogre::Radian(epsilon)))
	{		
		Ogre::Quaternion q(Ogre::Radian(limit), _axisInA);
		
		// reverse the rotation if new distance would be more than the old one
		vl::scalar new_dist = distance(current_q*q, target_q);
		if(dist < new_dist)
		{ q = q.Inverse(); }

		// Constraint is free if upper limit is less than lower limit
		if( !(_upper_limit < _lower_limit) )
		{
			Ogre::Quaternion lower_lim_q(_link->getInitialTransform()*Quaternion(_lower_limit, _axisInA));
			Ogre::Quaternion upper_lim_q(_link->getInitialTransform()*Quaternion(_upper_limit, _axisInA));
			vl::scalar lower_lim_dist = distance(current_q*q, lower_lim_q);
			vl::scalar upper_lim_dist = distance(current_q*q, upper_lim_q);

			// @fixme this allows for shooting past the limit if the speed is high enough
			// also this is a pretty dirty hack to handle the limits, we need to
			// have the direction of rotation here also...
			// Though this does work acceptably when speed < 1.
			vl::scalar eps = epsilon*_speed.valueRadians()*10;
			bool lower_limit_achieved = vl::equal(lower_lim_dist, vl::scalar(0), eps);
			bool upper_limit_achieved = vl::equal(upper_lim_dist, vl::scalar(0), eps);

			if(lower_limit_achieved || upper_limit_achieved)
			{
				q = Ogre::Quaternion::IDENTITY;
			}
		}
		// Only one point where the transformation occurs this ensures that
		// there will never be more than one transformation
		if(q != Ogre::Quaternion::IDENTITY)
		{
			_link->rotate(q);
		}
	}
}

/// ------------------------------ Private -----------------------------------
vl::HingeConstraint::HingeConstraint(KinematicBodyRefPtr rbA, KinematicBodyRefPtr rbB, Transform const &worldFrame)
	: Constraint(rbA, rbB, worldFrame)
	, _lower_limit()
	, _upper_limit()
	, _axisInA(0, 0, 1)
	, _angle()
	, _actuator(false)
	, _target()
	, _speed(Ogre::Degree(30))
{}
