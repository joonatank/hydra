/**	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-07
 *	@file input/serial_joystick.hpp
 *
 *	This file is part of Hydra a VR game engine.
 *
 *	Serial joystick interface for custom Arduino setup
 */

/**	Specifications for communications
 *	Designed for Arduino but can be used with anything that has similar interface.
 *
 *	Used baud rate 9600 in the Arduino
 * 
 *	Messages
 *	[MSG_READ_JOYSTICK]
 *	Request the joystick data from Arduino
 *
 *	[MSG_READ_JOYSTICK | N [ANALOG_ID CHANNEL DATA_HIGH DATA_LOW] | M [DIGITAL_ID CHANNEL DATA]]
 *	Response from Arduino
 *	Contains N analog channels and M digital channels every
 *	The order of the analog and digital channels is not significant
 *	The data for analog channel is 8 bits for id, 8 bits for channel, 
 *	8 bits for data high and 8 bits for data low,
 *	data is 10-bit integer (unsigned).
 *	The data for digital channel is 8 bits for id, 8 bits for channel and 8 bits for data,
 *	data is boolean (1 or 0).
 */

#ifndef HYDRA_INPUT_SERIAL_JOYSTICK_HPP
#define HYDRA_INPUT_SERIAL_JOYSTICK_HPP

#include <boost/signal.hpp>

// Interface
#include "input.hpp"

#include "math/math.hpp"

// Communications proto
#include "base/serial.hpp"

// Event stucture
#include "joystick_event.hpp"

namespace vl
{

// Serial message ids
const int MSG_READ_JOYSTICK = 75;

const uint8_t ANALOG_ID = 0;
const uint8_t DIGITAL_ID = 1;

// @todo change to vl::scalar
inline vl::scalar convert_analog(uint16_t data)
{
	return ((double)data)/(1024.0/2) - 1.0;
}

class SerialJoystick : public InputDevice
{
	typedef boost::signal<void (JoystickEvent const &)> OnValueChanged;
public :

	SerialJoystick(std::string const &device);

	virtual void mainloop(void);

	int doOnValueChanged(OnValueChanged::slot_type const &slot)
	{
		_signal.connect(slot);
		return 1;
	}

	static SerialJoystickRefPtr create(std::string const &dev)
	{
		SerialJoystickRefPtr joy(new SerialJoystick(dev));
		return joy;
	}

	void add_handler(SerialJoystickHandlerRefPtr handler);

private :
	// New parse function that creates the event structure
	JoystickEvent _parse(std::vector<char> msg, size_t bytes);

	void _write_data(void);

	OnValueChanged _signal;
	Serial _serial;

	std::vector<SerialJoystickHandlerRefPtr> _handlers;

};	// Class SerialJoystickReader

struct SerialJoystickHandler
{
	SerialJoystickHandler(void) {}

	virtual ~SerialJoystickHandler(void) {}

	virtual void execute(JoystickEvent const &evt) = 0;

};

}	// namespace vl

#endif	// HYDRA_INPUT_SERIAL_JOYSTICK_HPP
