
#include "constraints_handlers.hpp"

#include "constraints.hpp"


void
vl::ConstraintJoystickHandler::set_axis_constraint(int axis, ConstraintRefPtr constraint)
{
	set_axis_constraint(axis, -1, constraint);
}

void
vl::ConstraintJoystickHandler::set_axis_constraint(int axis, int button, ConstraintRefPtr constraint)
{
	AxisConstraintElem elem(axis, button, constraint);

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
		x_iter->constraint->setVelocity(_velocity_multiplier*evt.axis_x);
	}
	if(y_iter != _constraint_map.end())
	{
		y_iter->constraint->setVelocity(_velocity_multiplier*evt.axis_y);
	}
	if(z_iter != _constraint_map.end())
	{
		z_iter->constraint->setVelocity(_velocity_multiplier*evt.axis_z);
	}
}

vl::ConstraintJoystickHandlerRefPtr
vl::ConstraintJoystickHandler::create(void)
{
	ConstraintJoystickHandlerRefPtr handler(new ConstraintJoystickHandler);
	return handler;
}
