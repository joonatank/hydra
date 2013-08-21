/**
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-09
 *	@file input/joystick.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

#ifndef HYDRA_INPUT_JOYSTICK_HPP
#define HYDRA_INPUT_JOYSTICK_HPP

#include "input.hpp"

#include <boost/signal.hpp>

// Event stucture
#include "serial_joystick_event.hpp"

namespace vl
{

class Joystick : public InputDevice
{
	typedef boost::signal<void (SerialJoystickEvent const &)> OnValueChanged;

public :
	Joystick(void);

	virtual ~Joystick(void) {}

	int addListener(OnValueChanged::slot_type const &slot)
	{
		_signal.connect(slot);
		return 1;
	}

	/// @brief set what values are considered zero (percentage)
	/// @param size what is still considered as zero, 1 would mean everything
	void set_zero_size(vl::scalar size)
	{ _zero_size = size; }

	vl::scalar get_zero_size(void)
	{ return _zero_size; }

	void add_handler(JoystickHandlerRefPtr handler);

	void remove_handler(JoystickHandlerRefPtr handler);

	static JoystickRefPtr create(void)
	{
		JoystickRefPtr joy(new Joystick());
		return joy;
	}

	/// @brief called from event manager to update the event value
	void _update(SerialJoystickEvent const &evt);

protected :
	std::vector<JoystickHandlerRefPtr> _handlers;

	OnValueChanged _signal;

	vl::scalar _zero_size;

};	// class GameJoystick

struct JoystickHandler
{
	JoystickHandler(void) {}

	virtual ~JoystickHandler(void) {}

	virtual void execute(SerialJoystickEvent const &evt) = 0;

};	// struct JoystickHandler

}	// namespace vl

#endif	// HYDRA_INPUT_JOYSTICK_HPP
