/**	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-07
 *	@file input/serial_joystick.cpp
 *
 *	This file is part of Hydra a VR game engine.
 *
 *	Serial joystick interface for custom Arduino setup
 */

// Interface
#include "serial_joystick.hpp"

#include "base/timer.hpp"
// Necessary for waiting the controller to respond
#include "base/sleep.hpp"

const int BAUD_RATE = CBR_128000;

/// --------------------------------- Public ---------------------------------
vl::SerialJoystick::SerialJoystick(std::string const &device)
	: _serial(device, BAUD_RATE)
{
	// If necessary to decrease the timeouts be sure to increase baud rate also
	// on both arduino and here. Otherwise part of the message is lost.
	_serial.set_read_timeout(10, 5);
}

void
vl::SerialJoystick::mainloop(void)
{
	JoystickEvent evt;
	bool res = _request_data(evt);
	while(!res)
	{ res = _read_data(evt); }

	evt -= _zero;
	evt.clip_zero(_zero_size);

	// @todo should check that the data is valid new data
	// old signal system
	_signal(evt);

	for(std::vector<JoystickHandlerRefPtr>::iterator iter = _handlers.begin();
		iter != _handlers.end(); ++iter)
	{
		(*iter)->execute(evt);
	}
}

void
vl::SerialJoystick::calibrate_zero(void)
{
	if(!_request_data(_zero))
	{
		vl::msleep(1);
		// @todo add sleep
		while(!_read_data(_zero))
		{
			vl::msleep(1);
		}
	}

	std::clog << "Zero calibrated." << std::endl;

	// Request more data for the next read operation
	JoystickEvent evt;
	_request_data(evt);
}

/// --------------------------------- Private --------------------------------
vl::JoystickEvent
vl::SerialJoystick::_parse(std::vector<char> msg, size_t bytes)
{
	/// Parse the message
	assert(msg.at(0) == MSG_READ_JOYSTICK);
		
	JoystickEvent evt;
	int b = 1;
	while(b < bytes)
	{
		uint8_t id(msg.at(b));
		b += 1;
		if(ANALOG_ID == id)
		{
			uint16_t channel(uint8_t(msg.at(b)));
			uint8_t high_byte(msg.at(b+1));
			uint8_t low_byte(msg.at(b+2));
			uint16_t data = (high_byte << 8) | low_byte;
			b += 3;
			double analog = convert_analog(data);
				
			if(channel == 0)
			{
				evt.axis_x = analog;
			}
			else if(channel == 1)
			{
				evt.axis_y = analog;
			}
			else if(channel == 2)
			{
				evt.axis_z = analog;
			}
			else
			{
				std::cerr << "Unkown analog axis : " << channel << std::endl;
			}
		}
		else if(DIGITAL_ID == id)
		{
			uint8_t channel(uint8_t(msg.at(b)));
			uint8_t data(uint8_t(msg.at(b+1)));
			b += 2;
			evt.setButtonDown(channel, (bool)data);
		}
		else
		{
			// @todo this should throw as the message is incorrect
			std::cerr << "Not analog or digital channel." << std::endl;
		}
	}

	return evt;
}


bool
vl::SerialJoystick::_request_data(vl::JoystickEvent &evt)
{
	// The reading and request are switched because otherwise we don't have the
	// buffer filled when read.
	// The cleaner way to handle this would be to use message sizes
	// and custom buffer that is converted to JoyEvent when complete message
	// is received.

	// Read old data
	bool res = _read_data(evt);

	// Request new data
	std::vector<char> buf(128);
	buf.at(0) = char(MSG_READ_JOYSTICK);
	size_t bytes = _serial.write(buf, 1);
	// @todo replace by throwing
	if(bytes != 1)
	{
		std::cerr << "Something fishy : wrote " << bytes << " instead of 1." << std::endl;
	}

	return res;
}

bool
vl::SerialJoystick::_read_data(vl::JoystickEvent &evt)
{
	// Read data
	std::vector<char> buf(128);
	// @todo fix the hard coded number of bytes with arduino writing the
	// number first in the sequence
	size_t bytes = _serial.read(buf, 18);

	if(bytes > 0)
	{
		// Needs to throw because we can't handle partial messages
		if(bytes != 18)
		{
			std::stringstream ss;
			ss << "Incorrect number of bytes. Read " << bytes << " bytes." << std::endl;
			BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(ss.str()));
		}

		evt = _parse(buf, bytes);
		return true;
	}
	else
	{ return false; }
}
