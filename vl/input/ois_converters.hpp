/**	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-09
 *	@file input/ois_converters.hpp
 *
 *	This file is part of Hydra a VR game engine.
 *
 *	Converters from OIS event structures to Hydra event structures.
 */

#ifndef HYDRA_INPUT_OIS_CONVERS_HPP
#define HYDRA_INPUT_OIS_CONVERS_HPP

#include "joystick_event.hpp"

#include <OIS/OISJoyStick.h>

namespace vl
{

inline JoystickEvent
convert_ois_to_hydra(OIS::JoyStickEvent const &evt)
{
	// Does an axis swap for joysticks, not tested with gamepads
	// Assumes that MAX_AXIS == -MIN_AXIS
	double x = ((double)evt.state.mAxes[2].abs) / OIS::JoyStick::MAX_AXIS;
	double y = ((double)evt.state.mAxes[1].abs) / OIS::JoyStick::MAX_AXIS;
	double z = ((double)evt.state.mAxes[0].abs) / OIS::JoyStick::MAX_AXIS;

	JoystickEvent e;
	e.axis_x = x;
	e.axis_y = y;
	e.axis_z = z;

	for(size_t i = 0; i < evt.state.mButtons.size(); ++i)
	{
		e.setButtonDown(i, evt.state.mButtons.at(i));
	}

	return e;
}

}	// namespace vl

#endif	// HYDRA_INPUT_OIS_CONVERS_HPP
