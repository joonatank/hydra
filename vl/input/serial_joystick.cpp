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

const int BAUD_RATE = CBR_9600;

/// --------------------------------- Public ---------------------------------
vl::SerialJoystick::SerialJoystick(std::string const &device)
	: _serial(device, BAUD_RATE)
{}

void
vl::SerialJoystick::mainloop(void)
{
	vl::timer t;
	// Get new data
	// Write a request to the serial port for reading analog channel 0
	std::vector<char> buf(128);
	buf.at(0) = char(MSG_READ_JOYSTICK);
	size_t bytes = _serial.write(buf, 1);
	// @todo replace by throwing
	if(bytes != 1)
	{
		std::cerr << "Something fishy : wrote " << bytes << " instead of 1." << std::endl;
	}
	vl::time ser_write_t = t.elapsed();
	t.reset();

	bytes = _serial.read(buf, 128);
	vl::time ser_read_t = t.elapsed();
	t.reset();
	std::clog << "Read " << bytes << " from serial : took " << ser_read_t << std::endl;

	JoystickEvent evt = _parse(buf, bytes);
	vl::time evt_parse_t = t.elapsed();
	t.reset();

	// old signal system
	_signal(evt);
	vl::time signal_t = t.elapsed();
	t.reset();

	for(std::vector<SerialJoystickHandlerRefPtr>::iterator iter = _handlers.begin();
		iter != _handlers.end(); ++iter)
	{
		(*iter)->execute(evt);
	}
	vl::time handler_t = t.elapsed();

	std::clog << "vl::SerialJoystick::mainloop : serial write took "
		<< ser_write_t << " : serial read took " << ser_read_t 
		<< " : event parsing took " << evt_parse_t << " : signal took " << signal_t
		<< " : handlers took " << handler_t << std::endl;
}


void
vl::SerialJoystick::add_handler(vl::SerialJoystickHandlerRefPtr handler)
{
	// @tood check that it doesn't exist already
	std::vector<SerialJoystickHandlerRefPtr>::iterator iter = std::find(_handlers.begin(), _handlers.end(), handler);
	if(iter == _handlers.end())
	{ _handlers.push_back(handler); }
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
