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

#include <stdint.h>

#include "math/math.hpp"

#include "base/serial.hpp"

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


struct JoystickReaderCallback 
{
	virtual void valueChanged(JoystickEvent const &evt) = 0;
};

struct PrintJoystickCallback : public JoystickReaderCallback
{
	PrintJoystickCallback(std::ostream &os)
		: _stream(os)
	{}

	virtual void valueChanged(JoystickEvent const &evt)
	{
		std::cout << "New joystick values : " << evt << std::endl;
	}

	std::ostream &_stream;
};

class SerialJoystickReader
{
public :
	SerialJoystickReader(std::string const &device);

	// New parse function that creates the event structure
	JoystickEvent _parse(std::vector<char> msg, size_t bytes);

	void mainloop(void);

	void setCallback(JoystickReaderCallback *cb)
	{ _cb = cb; }

	JoystickReaderCallback *getCallback(void) const
	{ return _cb; }

private :
	JoystickReaderCallback *_cb;
	Serial _serial;

};	// Class SerialJoystickReader

}	// namespace vl

#endif	// HYDRA_INPUT_SERIAL_JOYSTICK_HPP
