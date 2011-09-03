/**	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-09
 *	@file input/input.cpp
 *
 *	This file is part of Hydra a VR game engine.
 *
 *	Basic input interface.
 */

#include "input/input.hpp"

void
vl::Joystick::add_handler(vl::JoystickHandlerRefPtr handler)
{
	// @tood check that it doesn't exist already
	std::vector<JoystickHandlerRefPtr>::iterator iter = std::find(_handlers.begin(), _handlers.end(), handler);
	if(iter == _handlers.end())
	{ _handlers.push_back(handler); }
}
