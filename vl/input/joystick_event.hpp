/**
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-07
 *	@file input/joystick_event.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

/**	Joystick Event structure, used by the serial joystick and will later be
 *	expanded to cover all joysticks e.g. usb.
 */


#ifndef HYDRA_INPUT_JOYSTICK_EVENT_HPP
#define HYDRA_INPUT_JOYSTICK_EVENT_HPP

// Necessary for ostream implementation
#include <iostream>

// Necessary for vl::scalar and uintX_t types
#include "math/math.hpp"

namespace vl
{

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

	int firstButtonDown(void) const
	{
		if(buttons & BUTTON0) { return 0; }
		else if(buttons & BUTTON1) { return 1; }
		else if(buttons & BUTTON2) { return 2; }
		else if(buttons & BUTTON3) { return 3; }
		else if(buttons & BUTTON4) { return 4; }
		else if(buttons & BUTTON5) { return 5; }
		else if(buttons & BUTTON6) { return 6; }
		else if(buttons & BUTTON7) { return 7; }
		else { return -1; }
	}

	JoystickEvent &
	operator-=(JoystickEvent const &a)
	{
		axis_x -= a.axis_x;
		axis_y -= a.axis_y;
		axis_z -= a.axis_z;
		return *this;
	}

	void clip_zero(vl::scalar zero_size)
	{
		if(std::abs(axis_x) < zero_size)
		{ axis_x = 0; }
		if(std::abs(axis_y) < zero_size)
		{ axis_y = 0; }
		if(std::abs(axis_z) < zero_size)
		{ axis_z = 0; }
	}

	vl::scalar axis_x;
	vl::scalar axis_y;
	vl::scalar axis_z;

	uint32_t buttons;

};	// struct JoystickEvent

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

inline JoystickEvent
operator-(JoystickEvent const &a, JoystickEvent const &b)
{
	JoystickEvent res(a);
	res -= b;
	return res;
}

inline bool
operator==(JoystickEvent const &a, JoystickEvent const &b)
{
	return( a.axis_x == b.axis_x && a.axis_y == b.axis_y
		&& a.axis_z == b.axis_z && a.buttons == b.buttons);
}

inline bool
operator!=(JoystickEvent const &a, JoystickEvent const &b)
{
	return !(a==b);
}

}	// namespace vl

#endif	// HYDRA_INPUT_JOYSTICK_EVENT_HPP
