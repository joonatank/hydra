/**	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-07
 *	@file input/joystick_event.hpp
 *
 *	This file is part of Hydra a VR game engine.
 *
 *	Joystick Event structure, used by the serial joystick and will later be
 *	expanded to cover all joysticks e.g. usb.
 */


#ifndef HYDRA_INPUT_JOYSTICK_EVENT_HPP
#define HYDRA_INPUT_JOYSTICK_EVENT_HPP

// Necesasry for uint8_t and uint32_t
#include <stdint.h>
// Necessary for ostream implementation
#include <iostream>

// Necessary for vl::scalar
#include "math/math.hpp"

struct JoystickEvent
{
	JoystickEvent(void)
		: axis_x(0)
		, axis_y(0)
		, axis_z(0)
		, buttons(0)
	{}

	~JoystickEvent(void) {}

	enum BUTTON
	{
		BUTTON0 = (1 << 0),
		BUTTON1 = (1 << 1),
		BUTTON2 = (1 << 2),
		BUTTON3 = (1 << 3),
		BUTTON4 = (1 << 4),
		BUTTON5 = (1 << 5),
		BUTTON6 = (1 << 6),
		BUTTON7 = (1 << 7),
	};

	// @brief get the button state based on the button index, from 0 to 8
	// This is quite dangerous function as the number could be casted to
	// BUTTON also, which would be terrible idea as BUTTON is the hex code
	// in the button structure.
	bool isButtonDown(size_t index) const
	{
		return(buttons & (1 << index));
	}

	bool isButtonDown(BUTTON bt) const
	{
		return(buttons & bt);
	}

	void setButtonDown(BUTTON index, bool down)
	{
		setButtonDown((uint8_t)index, down);
	}

	void setButtonDown(uint8_t index, bool down)
	{
		if(down)
		{ buttons |= (1 << index); }
		else
		{ buttons &= ~(1 << index); }
	}

	vl::scalar axis_x;
	vl::scalar axis_y;
	vl::scalar axis_z;

	uint32_t buttons;
};

inline std::ostream &
operator<<(std::ostream &os, JoystickEvent const &evt)
{
	os << "Axis = (" << evt.axis_x << ", " << evt.axis_y << ", " << evt.axis_z << ")" << std::endl;
	// @todo add buttons;
	for(size_t i = 0; i < 8; ++i)
	{
		os << "Button " << i << " : " << evt.isButtonDown(i) << std::endl;
	}

	return os;
}

#endif	// HYDRA_INPUT_JOYSTICK_EVENT_HPP
