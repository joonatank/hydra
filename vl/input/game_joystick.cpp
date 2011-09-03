/**	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-09
 *	@file input/game_joystick.cpp
 *
 *	This file is part of Hydra a VR game engine.
 *
 *	Game joystick interface for regular computer joysticks.
 */

#include "game_joystick.hpp"

vl::GameJoystick::GameJoystick(void)
{
}

void
vl::GameJoystick::mainloop(void)
{
	// empty mainloop because _update handles value changes
}

void
vl::GameJoystick::_update(vl::JoystickEvent const &evt)
{
	// @todo should check that the data is valid new data
	// old signal system
	_signal(evt);

	for(std::vector<JoystickHandlerRefPtr>::iterator iter = _handlers.begin();
		iter != _handlers.end(); ++iter)
	{
		(*iter)->execute(evt);
	}
}
