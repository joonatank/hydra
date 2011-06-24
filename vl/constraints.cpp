/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-06
 *	@file constraints.cpp
 *
 *	This file is part of Hydra a VR game engine.
 *
 *	Defines basic non-physical constraints. These are based on the common
 *	constraints in physics engines, but do not do the dynamics simulation.
 */

#include "constraints.hpp"

#include "scene_node.hpp"

std::ostream &
vl::operator<<(std::ostream &os, vl::Constraint const &c)
{
	os << "Constraint with bodies : " << c._bodyA->getName() << " and " 
		<< c._bodyB->getName() << "\n"
		<< " Transformation = " << c._getLink()->getTransform()
		<< " Initial transformation = " << c._getLink()->getInitialTransform()
		<< std::endl;


	return os;
}

/// ------------------------------ Constraint --------------------------------
/// ------------------------------ Public ------------------------------------

/// ------------------------------ Protected ---------------------------------
vl::Constraint::Constraint(SceneNodePtr rbA, SceneNodePtr rbB, vl::Transform const &worldFrame)
	: _bodyA(rbA)
	, _bodyB(rbB)
{
	assert(_bodyA);
	assert(_bodyB);

	vl::Transform wtA(_bodyA->getWorldTransform());
	wtA.invert();

	vl::Transform wtB(_bodyB->getWorldTransform());
	wtB.invert();

	_current_local_frame_a = wtA*worldFrame;
	_current_local_frame_b = wtB*worldFrame;
}

void 
vl::Constraint::_setLink(vl::animation::LinkRefPtr link)
{
	assert(!_link && link);
	_link = link;

	// Shouldn't change parent's transformation
	// This problem only arises because there is a problem when setting
	// the transformation of bodyB
	if( !_link->getParent()->getWorldTransform().isIdentity() &&
		_link->getParent()->getWorldTransform() != _bodyA->getWorldTransform() )
	{}
	else
	{
		_link->getParent()->setWorldTransform(_bodyA->getWorldTransform());
	}
	
	// needs to be set before child to get the correct local matrix for child
	_link->setTransform(_current_local_frame_b);
	_link->setInitialState();

	Transform wt(_bodyB->getWorldTransform());
	_link->getChild()->setWorldTransform(wt);
}

/// ------------------------------ FixedConstraint ---------------------------
/// ------------------------------ Public ------------------------------------
void
vl::FixedConstraint::_proggress(vl::time const &t)
{}

/// ------------------------------ Private -----------------------------------
vl::FixedConstraint::FixedConstraint(SceneNodePtr rbA, SceneNodePtr rbB, Transform const &worldFrame)
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
vl::scalar
vl::SliderConstraint::getLowerLimit(void) const
{
	return _lower_limit;
}
	
void
vl::SliderConstraint::setLowerLimit(vl::scalar lowerLimit)
{
	_lower_limit = lowerLimit;
}

vl::scalar
vl::SliderConstraint::getUpperLimit(void) const
{
	return _upper_limit;
}
	
void
vl::SliderConstraint::setUpperLimit(vl::scalar upperLimit)
{
	_upper_limit = upperLimit;
}

// Motor
void
vl::SliderConstraint::enableMotor(bool enable)
{
	_motor_enabled = enable;
}

bool
vl::SliderConstraint::getMotorEnabled(void) const
{
	return _motor_enabled;
}

void 
vl::SliderConstraint::addTarget(vl::scalar target_pos_addition)
{_target_position += target_pos_addition; }

void 
vl::SliderConstraint::setMotorTarget(vl::scalar target_pos)
{
	_target_position = target_pos; 
}

vl::scalar 
vl::SliderConstraint::getMotorTarget(void)
{
	return _target_position;
}

void
vl::SliderConstraint::addMotorVelocity(vl::scalar velocity)
{
	_velocity += velocity;
}

void
vl::SliderConstraint::setMotorVelocity(vl::scalar velocity)
{
	_velocity = velocity;
}

vl::scalar
vl::SliderConstraint::getMotorVelocity(void) const
{
	return _velocity;
}

void
vl::SliderConstraint::_proggress(vl::time const &t)
{	
	if(!_link)
	{ return; }

	/// @todo add configurable axis using _axisInA
	/// @todo starting position is in Frame A, so we should fix the offset from Frame B
	/// that is in the start, 
	/// as WT*_start_local_frame_a.position.y != WT*_start_local_frame_b.position.y
	/// which causes the zero position for Frame B be in the origin of Frame A
	/// but this does not change it before the target has been changed.

	vl::scalar translate = 0;
	vl::scalar pos = _link->getTransform().position.y - _link->getInitialTransform().position.y;
	/// @todo add tolerance
	if(_motor_enabled && _target_position != pos)
	{
		/// Axis is either not limited (lower limit is greater than upper)
		/// or if it's current position is within the limits
		/// @todo should we update the target position based on limits, 
		/// so that there is no impossible target

		/// @todo how to handle negative targets, should the user use negative
		/// velocity or is it only speed and we need to decide which way to go?
		vl::scalar s = vl::sign(_target_position - pos);
		translate = s*abs(_velocity) * double(t);
		
		/// Clamp the translation
		if(translate < 0)
		{ translate = vl::clamp(translate, _target_position-pos, vl::scalar(0)); }
		else
		{ translate = vl::clamp(translate, vl::scalar(0), _target_position-pos); }
	}
	else
	{
		// @todo this should handle free movemenf following the A object
	}
	
	/// Move the child body
	/// Wether the joint is limited or not
	/// @todo this is common for all joints, though the constraint type linear/angular changes
	bool free = _lower_limit > _upper_limit;
	/// Check constraints
	if(!free)
	{
		if(translate < 0)
		{ free = (pos > _lower_limit); }
		else if(translate > 0)
		{ free = (pos < _upper_limit); }

		// Clamp only if not free
		// @todo should we clamp the target instead of the result? 
		// then again any subsequent change in limits would not get the actuator moving
		translate = vl::clamp(translate, _lower_limit-pos, _upper_limit-pos);
		assert(translate+pos >= _lower_limit);
		assert(translate+pos <= _upper_limit);
	}

	if(free)
	{
		/// Update object B
		_link->getTransform().position.y += translate;
	}
}

/// ------------------------------ Private -----------------------------------
vl::SliderConstraint::SliderConstraint(SceneNodePtr rbA, SceneNodePtr rbB, Transform const &worldFrame)
	: Constraint(rbA, rbB, worldFrame)
	, _lower_limit(0)
	, _upper_limit(0)
	, _axisInA(0, 0, 1)
	, _motor_enabled(false)
	, _target_position(0)
	, _velocity(1)
{}

/// ------------------------------ HingeConstraint ---------------------------
/// ------------------------------ Public ------------------------------------	
void
vl::HingeConstraint::enableMotor(bool enableMotor)
{
	_motor_enabled = enableMotor;
}

bool
vl::HingeConstraint::getMotorEnabled(void) const
{
	return _motor_enabled;
}

void
vl::HingeConstraint::setMotorTarget(Ogre::Radian const &angle)
{
	_target = angle;
}

void 
vl::HingeConstraint::setMotorVelocity(Ogre::Radian const &dt)
{
	_velocity = dt;
}

Ogre::Radian const &
vl::HingeConstraint::getMotorTarget(void) const
{
	return _target;
}

Ogre::Radian const &
vl::HingeConstraint::getMotorDerivative(void) const
{
	return _velocity;
}

void
vl::HingeConstraint::setLowerLimit(Ogre::Radian const &lower)
{
	_lower_limit = lower;
}

Ogre::Radian const &
vl::HingeConstraint::getLowerLimit(void) const
{
	return _lower_limit;
}

void
vl::HingeConstraint::setUpperLimit(Ogre::Radian const &upper)
{
	_upper_limit = upper;
}

Ogre::Radian const &
vl::HingeConstraint::getUpperLimit(void) const
{
	return _upper_limit;
}

void
vl::HingeConstraint::setAxis(Ogre::Vector3 const &axisInA)
{
	_axisInA = axisInA;
}

Ogre::Vector3 const &
vl::HingeConstraint::getAxisInA(void) const
{
	return _axisInA;
}

Ogre::Vector3
vl::HingeConstraint::getAxisInWorld(void) const
{
	return _bodyA->getWorldTransform()*_axisInA;
}

Ogre::Radian const &
vl::HingeConstraint::getHingeAngle(void) const
{
	return _angle;
}

void
vl::HingeConstraint::_proggress(vl::time const &t)
{
	bool free = _upper_limit < _lower_limit;
	if(_motor_enabled)
	{
	}
	else
	{

	}

	// @todo update _current_local_frame_a
//	_current_local_frame_a.position.y += translate;
	//_update_bodies();
}

/// ------------------------------ Private -----------------------------------
vl::HingeConstraint::HingeConstraint(SceneNodePtr rbA, SceneNodePtr rbB, Transform const &worldFrame)
	: Constraint(rbA, rbB, worldFrame)
	, _lower_limit()
	, _upper_limit()
	, _axisInA(0, 0, 1)
	, _angle()
	, _motor_enabled(false)
	, _target()
	, _velocity(Ogre::Degree(30))
{}
