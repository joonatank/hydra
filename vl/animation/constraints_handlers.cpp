/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-06
 *	@file animation/constraints_handlers.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

#include "constraints_handlers.hpp"

#include "constraints.hpp"

/// ------------------------------ Public ------------------------------------
void
vl::ConstraintJoystickHandler::set_axis_constraint(ConstraintRefPtr constraint, 
	int axis, int button, bool inverted)
{
	AxisConstraintElem elem(axis, button, inverted, constraint);

	std::vector<AxisConstraintElem>::iterator iter= std::find(_constraint_map.begin(), _constraint_map.end(), elem);
	if(iter != _constraint_map.end())
	{
		*iter = elem;
	}
	else
	{
		_constraint_map.push_back(elem);
	}
}
	

void
vl::ConstraintJoystickHandler::execute(JoystickEvent const &evt)
{
	bool valid = true;

	// if the last or current event has all axes zero then it's always valid

	// if both current and last event has some axes other than zero
	// the event is only valid if the buttons are equal
	// Does not use axis z because it is throttle on game joysticks and
	// it does not exists for the serial joystick.
	if( !(evt.axis_x == 0 && evt.axis_y == 0) )
	{
		// This has the problem that if user presses one extra button
		// down that becames the first button. This will return invalid.
		// Then again should this be considered bug or a feature
		// depends on the use case.
		valid = (evt.firstButtonDown() == _last_event.firstButtonDown());
		// Problematic if we are already in movement either the movement
		// needs to be stopped or we need to use the last button values.
	}

	if(valid)
	{
		_apply_event(evt);
		_last_event = evt;
	}
	else
	{
		// Reset all axes
		// we could also use the last buttons and current axes for continuing
		// the movement, but this way is safer.
		JoystickEvent e;
		e.buttons = _last_event.buttons;
		_apply_event(e);
	}

}

vl::ConstraintJoystickHandlerRefPtr
vl::ConstraintJoystickHandler::create(void)
{
	ConstraintJoystickHandlerRefPtr handler(new ConstraintJoystickHandler);
	return handler;
}

void 
vl::ConstraintJoystickHandler::_apply_event(JoystickEvent const &evt)
{
	AxisConstraintElem elem_x(0, evt.firstButtonDown());
	AxisConstraintElem elem_y(1, evt.firstButtonDown());
	AxisConstraintElem elem_z(2, evt.firstButtonDown());
	std::vector<AxisConstraintElem>::iterator x_iter= std::find(_constraint_map.begin(), _constraint_map.end(), elem_x);
	std::vector<AxisConstraintElem>::iterator y_iter= std::find(_constraint_map.begin(), _constraint_map.end(), elem_y);
	std::vector<AxisConstraintElem>::iterator z_iter= std::find(_constraint_map.begin(), _constraint_map.end(), elem_z);

	/// @todo add fallback to no buttons down
	/// @todo reset the velocities for anything not present
	/// for example button was pressed is no longer so set it's velocity to zero
	if(x_iter != _constraint_map.end())
	{
		vl::scalar sign = 1;
		if(x_iter->inverted)
		{ sign = -1; }
		vl::scalar velocity = sign*_velocity_multiplier*evt.axis_x;
		x_iter->constraint->setVelocity(velocity);
	}
	if(y_iter != _constraint_map.end())
	{
		vl::scalar sign = 1;
		if(y_iter->inverted)
		{ sign = -1; }
		vl::scalar velocity = sign*_velocity_multiplier*evt.axis_y;
		y_iter->constraint->setVelocity(velocity);
	}
	if(z_iter != _constraint_map.end())
	{
		vl::scalar sign = 1;
		if(z_iter->inverted)
		{ sign = -1; }
		z_iter->constraint->setVelocity(sign*_velocity_multiplier*evt.axis_z);
	}

}

/// ------------------------------ Protected ---------------------------------
vl::ConstraintJoystickHandler::ConstraintJoystickHandler(void)
	: _velocity_multiplier(1.0)
{}
