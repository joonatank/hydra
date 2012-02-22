/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-07
 *	@file input/serial_joystick.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

// Interface
#include "serial_joystick.hpp"

#include "base/time.hpp"
// Necessary for waiting the controller to respond
#include "base/sleep.hpp"

#include "logger.hpp"

const int BAUD_RATE = CBR_128000;

/// --------------------------------- Public ---------------------------------
vl::SerialJoystick::SerialJoystick(std::string const &device)
	: _serial(device, BAUD_RATE)
	, _request_sent(false)
	, _incomplete_joystick_msg(false)
{
	// If necessary to decrease the timeouts be sure to increase baud rate also
	// on both arduino and here. Otherwise part of the message is lost.
	_serial.set_read_timeout(100, 20);
}

void
vl::SerialJoystick::mainloop(void)
{
	std::vector<JoystickEvent> events;

	if(_joysticks.size() == 0)
	{
		std::string err_msg("SerialJoystick shouldn't be able to be created without any joysticks.");
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(err_msg));
	}
	
	bool res = _read_multi_data(events, false);
	if(!res)
	{
		assert(events.empty());
	}
	else
	{
		// @todo this will use same zero for all joysticks
		// even though they differ
		for(size_t i = 0; i < events.size(); ++i)
		{
			assert(_zeros.size() > i);
			events.at(i) -= _zeros.at(i);
			assert(_joysticks.size() > i);
			_joysticks.at(i)->_update(events.at(i));
		}
	}

	// Request more data for next read
	_request_multi_data();
}

void
vl::SerialJoystick::initialise(void)
{
	_serial.set_read_timeout(1000, 200);
	std::vector<JoystickEvent> events;

	_request_multi_data();
	vl::msleep(1);
	if(!_read_multi_data(events, true))
	{
		BOOST_THROW_EXCEPTION(vl::exception());
	}

	/// Resize the joystick array
	for(size_t i = _joysticks.size(); i < events.size(); ++i)
	{
		_joysticks.push_back(Joystick::create());
	}
	_joysticks.resize(events.size());

	if(_joysticks.size() == 0)
	{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("No joysticks in the valid serial connection.")); }
}

void
vl::SerialJoystick::calibrate_zero(void)
{
	_serial.set_read_timeout(1000, 200);
	// Needs to be zeroed because the events are pushed back
	_zeros.clear();

	_request_multi_data();
	vl::msleep(1);
	if(!_read_multi_data(_zeros, true))
	{
		BOOST_THROW_EXCEPTION(vl::exception());
	}

	// Send a new request for next call
	_request_multi_data();

	std::clog << "Zero calibrated." << std::endl;

	assert(_zeros.size() == _joysticks.size());

	// Request more data for the next read operation
	_request_multi_data();
}

/// --------------------------------- Private --------------------------------
vl::JoystickEvent
vl::SerialJoystick::_parse(std::vector<char> msg, size_t bytes, size_t offset)
{
	size_t end_index = offset+bytes;

	/// Parse the message
	if(msg.at(offset++) != MSG_JOYSTICK_START)
	{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Joystick message does not have MSG_JOYSTICK_START")); }

	JoystickEvent evt;

	while(offset < end_index)
	{
		uint8_t id(msg.at(offset++));
		if(ANALOG_ID == id)
		{
			uint16_t channel(uint8_t(msg.at(offset++)));
			uint8_t high_byte(msg.at(offset++));
			uint8_t low_byte(msg.at(offset++));
			uint16_t data = (high_byte << 8) | low_byte;
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
			uint8_t channel(uint8_t(msg.at(offset++)));
			uint8_t data(uint8_t(msg.at(offset++)));
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

void
vl::SerialJoystick::_request_multi_data(void)
{
	// The reading and request are switched because otherwise we don't have the
	// buffer filled when read.
	// The cleaner way to handle this would be to use message sizes
	// and custom buffer that is converted to JoyEvent when complete message
	// is received.

	// Request new data
	if(!_request_sent)
	{
		std::vector<char> buf(128);
		buf.at(0) = MSG_READ_MULTIPLE_JOYSTICKS;
		size_t bytes = _serial.write(buf, 1);

		if(bytes != 1)
		{
			std::stringstream ss;
			ss << "Something fishy : wrote " << bytes << " bytes instead of 1." << std::endl;
			BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(ss.str()));
		}

		_request_sent = true;
	}
}

bool
vl::SerialJoystick::_read_multi_data(std::vector<vl::JoystickEvent> &evt, bool blocking)
{
	// Read data
	std::vector<char> buf(128);
	bool ret_val = false;
	size_t bytes = 0;

	/// Finish an incomplete message
	if(_incomplete_joystick_msg)
	{
		bytes = _serial.read(buf, _joystick_msg_size, blocking);
			
		if(bytes)
		{
			// This shouldn't ever happen
			if(bytes != _joystick_msg_size )
			{
				std::stringstream ss;
				ss << "Incorrect number of bytes. Read " << bytes 
					<< " bytes instead of " << _joystick_msg_size;
				BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(ss.str()));
			}
				
			_parse_joystick_msg(evt, buf);
			_incomplete_joystick_msg = false;
			_request_sent = false;
			ret_val = true;
			
			bytes = _serial.read(buf, 2, blocking);
		}
	}
	else
	{ bytes = _serial.read(buf, 2, blocking); }

	/// Process a new message
	while(bytes)
	{
		size_t offset = 0;
		assert(bytes == 2);
		uint16_t type = buf.at(offset++);
		switch(type) 
		{
			case MSG_READ_MULTIPLE_JOYSTICKS:
			{
				_joystick_msg_size = buf.at(offset++);

				// Read the real message
				bytes = _serial.read(buf, _joystick_msg_size, blocking);
				
				if(!bytes)
				{
					_incomplete_joystick_msg = true; 
				}
				else
				{
					// This shouldn't ever happen
					if(bytes != _joystick_msg_size )
					{
						std::stringstream ss;
						ss << "Incorrect number of bytes. Read " << bytes 
							<< " bytes instead of " << _joystick_msg_size;
						BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(ss.str()));
					}
				
					_parse_joystick_msg(evt, buf);
					_incomplete_joystick_msg = false;
					_request_sent = false;
					ret_val = true;
				}
			}
			break;

			case MSG_ERROR:
			{
				size_t msg_length = buf.at(offset++);
				bytes = _serial.read(buf, msg_length);
				offset = 0;
				assert(bytes == msg_length);

				size_t error_code = buf.at(offset++);
				uint16_t info = buf.at(offset++);
				std::stringstream error_msg;
				switch(error_code)
				{
				case ERR_INCORRECT_BYTES_WRITEN :
					error_msg << "ERR_INCORRECT_BYTES_WRITEN : bytes = " << info;
					break;
				case ERR_UNKNOWN_MSG :
					error_msg << "ERR_UNKNOWN_MSG : message id = " << info;
					break;
				default :
					error_msg << "Unkown error";
					break;
				}

				assert(buf.at(msg_length-1) == MSG_STOP);

				std::cout << vl::CRITICAL << error_msg.str() << std::endl;
			}
			break;

			default:
				std::cout << vl::CRITICAL << "Unknow Message " << type << " received from Serial." << std::endl;
		}
		
		if(!_incomplete_joystick_msg)
		{ bytes = _serial.read(buf, 2, blocking); }
	}

	return ret_val;
}

void
vl::SerialJoystick::_parse_joystick_msg(std::vector<vl::JoystickEvent> &evt, std::vector<char> msg)
{
	size_t offset = 0;

	// Check that there is a stop
	assert(msg.at(_joystick_msg_size-1) == MSG_STOP);

	uint16_t n_joysticks = msg.at(offset++);
	uint16_t joystick_size = msg.at(offset++);

	for(size_t i = 0; i < n_joysticks; ++i)
	{
		// check that all joysticks are valid
		assert(msg.at(offset) == MSG_JOYSTICK_START);

		JoystickEvent eve;
			
		eve = _parse(msg, joystick_size, offset);
		offset += joystick_size;
		evt.push_back(eve);
	}
}
