/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-07
 *	@file input/serial_joystick.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

/*
 *	Serial joystick interface for custom Arduino setup
 *	This is the concrete implemention used to update the Joystcik
 *	proxy which is the same for both Arduino joysticks and
 *	game joysticks.
 */

/**	Specifications for communications
 *	Designed for Arduino but can be used with anything that has similar interface.
 *
 *	Used baud rate 128000 in the Arduino
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

#include "joystick.hpp"

#include "math/math.hpp"

// Communications proto
#include "base/serial.hpp"

namespace vl
{

// Serial message ids
// Main message id (request - response)
// Single joystick
const uint8_t MSG_READ_JOYSTICK = 75;
// Multiple joysticks
const uint8_t MSG_READ_MULTIPLE_JOYSTICKS = 77;
// Separator for multiple joysticks
const uint8_t MSG_JOYSTICK_START = 40;
const uint8_t MSG_ERROR = 41;
const uint8_t MSG_STOP = 36;

// Error messages
const uint8_t ERR_INCORRECT_BYTES_WRITEN = 1;
const uint8_t ERR_UNKNOWN_MSG = 2;

const uint8_t ANALOG_ID = 0;
const uint8_t DIGITAL_ID = 1;

// @todo change to vl::scalar
inline vl::scalar convert_analog(uint16_t data)
{
	return ((double)data)/(1024.0/2) - 1.0;
}

class SerialJoystick
{
public :
	SerialJoystick(std::string const &device);

	virtual ~SerialJoystick(void) {}

	void mainloop(void);

	/// @brief checks that the connection is valid and the number of
	/// joysticks available.
	/// Creates the joysticks.
	void initialise(void);

	static SerialJoystickRefPtr create(std::string const &dev)
	{
		SerialJoystickRefPtr joy(new SerialJoystick(dev));
		joy->initialise();
		joy->calibrate_zero();
		return joy;
	}

	void calibrate_zero(void);

	size_t getNJoysticks(void) const
	{ return _joysticks.size(); }

	JoystickRefPtr getJoystick(size_t i)
	{ return _joysticks.at(i); }

private :

	JoystickEvent _parse(std::vector<char> msg, size_t bytes, size_t offset);

	void _request_multi_data(void);

	bool _read_multi_data(std::vector<vl::JoystickEvent> &evt, bool blocking = true);

	void _parse_joystick_msg(std::vector<vl::JoystickEvent> &evt, std::vector<char> msg);

	Serial _serial;

	// State wether request is sent without receiving an answer
	bool _request_sent;
	bool _incomplete_joystick_msg;
	size_t _joystick_msg_size;

	std::vector<JoystickRefPtr> _joysticks;

	std::vector<JoystickEvent> _zeros;

};	// Class SerialJoystickReader

}	// namespace vl

#endif	// HYDRA_INPUT_SERIAL_JOYSTICK_HPP
