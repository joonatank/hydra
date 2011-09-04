/**	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-07
 *	@file input/input.hpp
 *
 *	This file is part of Hydra a VR game engine.
 *
 *	Basic input interface.
 */

#ifndef HYDRA_INPUT_INPUT_HPP
#define HYDRA_INPUT_INPUT_HPP

#include <boost/signal.hpp>

// Event stucture
#include "joystick_event.hpp"

namespace vl
{

class InputDevice
{
public :
	virtual ~InputDevice(void) {}

	virtual void mainloop(void) = 0;

};	// class InputDevice

class Joystick : public InputDevice
{
	typedef boost::signal<void (JoystickEvent const &)> OnValueChanged;

public :
	virtual ~Joystick(void) {}

	int addListener(OnValueChanged::slot_type const &slot)
	{
		_signal.connect(slot);
		return 1;
	}

	void add_handler(JoystickHandlerRefPtr handler);

protected :
	std::vector<JoystickHandlerRefPtr> _handlers;

	OnValueChanged _signal;

};	// class Joystick

struct JoystickHandler
{
	JoystickHandler(void) {}

	virtual ~JoystickHandler(void) {}

	virtual void execute(JoystickEvent const &evt) = 0;

};	// struct JoystickHandler

}	// namespace vl

#endif	// HYDRA_INPUT_INPUT_HPP
