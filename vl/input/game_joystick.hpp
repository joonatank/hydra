/**	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-09
 *	@file input/game_joystick.hpp
 *
 *	This file is part of Hydra a VR game engine.
 *
 *	Game joystick interface for regular computer joysticks.
 */

#ifndef HYDRA_INPUT_GAME_JOYSTICK_HPP
#define HYDRA_INPUT_GAME_JOYSTICK_HPP

#include "input.hpp"

namespace vl
{

class GameJoystick : public Joystick
{
public :
	GameJoystick(void);

	virtual ~GameJoystick(void) {}

	virtual void mainloop(void);

	static GameJoystickRefPtr create(void)
	{
		GameJoystickRefPtr joy(new GameJoystick());
		return joy;
	}

	/// @brief called from event manager to update the event value
	void _update(JoystickEvent const &evt);

};	// class GameJoystick

}	// namespace vl

#endif	// HYDRA_INPUT_GAME_JOYSTICK_HPP
