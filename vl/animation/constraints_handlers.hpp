/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-06
 *	@file animation/constraints_handlers.hpp
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
 *	Contains Actions and Event handlers to use with the animation framework 
 *	constraints.
 */

#ifndef HYDRA_CONSTRAINTS_HANDLERS_HPP
#define HYDRA_CONSTRAINTS_HANDLERS_HPP

/// Base class
#include "input/serial_joystick.hpp"

namespace vl
{

struct ConstraintJoystickHandler : public JoystickHandler
{
	struct AxisConstraintElem
	{
		int axis;
		int button;
		ConstraintRefPtr constraint;
		bool inverted;

		AxisConstraintElem(int axis_, int button_, bool inverted_ = false, ConstraintRefPtr constraint_ = ConstraintRefPtr())
			: axis(axis_), button(button_), inverted(inverted_), constraint(constraint_)
		{}

		bool operator==(AxisConstraintElem const &elem) const
		{
			return(axis == elem.axis && button == elem.button);
		}
	};

	virtual ~ConstraintJoystickHandler(void) {}

	// @todo configuration which axes control which constraints
	// unified control for both slider and hinge constraints
	
	/// @param axis, axis which is used to control constraint 0 = x, 1 = y, 2 = z
	/// @param button, button that needs to be pressed for this constraint
	/// @param constraint, the constraint to control
	void set_axis_constraint(ConstraintRefPtr constraint, int axis, int button = -1, bool inverted = false);
	
	void set_velocity_multiplier(vl::scalar multi)
	{ _velocity_multiplier = vl::abs(multi); }

	vl::scalar get_velocity_multiplier(void) const
	{ return _velocity_multiplier; }

	virtual void execute(JoystickEvent const &evt);

	static ConstraintJoystickHandlerRefPtr create(void);
	
protected :
	ConstraintJoystickHandler(void);

	void _apply_event(JoystickEvent const &evt);

	vl::scalar _velocity_multiplier;

	JoystickEvent _last_event;

	std::vector<AxisConstraintElem> _constraint_map;
};

}	// namespace vl

#endif	// HYDRA_CONSTRAINTS_HANDLERS_HPP
