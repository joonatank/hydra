/**
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-07
 *	@file base/serial.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

/**
 *	Low level serial device reader.
 *	@todo add Linux implementation
 */

// Interface
#include "serial.hpp"

#include <iostream>

#include "exceptions.hpp"

vl::Serial::Serial(std::string const &dev_name, uint32_t baud_rate)
	: _hSerial(0)
	, _buffer_size(0)
{
	_buffer.resize(512);

	try {
		if(!dev_name.empty())
		{ open(dev_name, baud_rate); }
	}
	catch(...)
	{
		close();
	}
}

vl::Serial::~Serial(void)
{
	close();
}

void
vl::Serial::open(std::string const &dev_name, uint32_t baud_rate)
{
	_name = dev_name;
	_hSerial = CreateFile(dev_name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if(_hSerial==INVALID_HANDLE_VALUE)
	{
		std::stringstream ss;
		ss << "Serial port : " << _name << " : ";
		
		if(GetLastError()==ERROR_FILE_NOT_FOUND)
		{
			ss << "No such COM port.";
		}
		else
		{
			ss << "Unknow error.";
		}

		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(ss.str()));
	}

	// Set options
	DCB dcbSerialParams = {0};
	dcbSerialParams.DCBlength=sizeof(dcbSerialParams);
	if (!GetCommState(_hSerial, &dcbSerialParams)) 
	{
		//error getting state
		std::stringstream ss;
		ss << "Serial port : " << _name << " couldn't get the state.";
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(ss.str()));
	}

	dcbSerialParams.BaudRate = baud_rate;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity = NOPARITY;
	if(!SetCommState(_hSerial, &dcbSerialParams))
	{
		//error setting serial port state
		std::stringstream ss;
		ss << "Serial port : " << _name << " couldn't set the state.";
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(ss.str()));
	}

	// set some default timeouts so the application will not hang
	set_read_timeout(50, 5);
	set_write_timeout(100);
}

void
vl::Serial::close(void)
{
	if(_hSerial)
	{
		CloseHandle(_hSerial);
		_hSerial = 0;
	}
}

size_t
vl::Serial::read(std::vector<char> &buf, size_t n_bytes, bool blocking)
{
	if(n_bytes == 0)
	{ return 0; }

	if(buf.size() < n_bytes)
	{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Buffer size can not be less than bytes to read.")); }

	_copy_os_buffer();
	if(_buffer_size < n_bytes && blocking)
	{
		// do an extra blocking copy
		_copy_os_buffer(n_bytes - _buffer_size);
	}

	// @todo this should of course throw as we don't allow invalid messages
	// and assertion will not do in release version
	// this would come as a problem when there is a large message that can not
	// be completely read because of too low timeout values.
	// rather we should keep our buffer intanct and return a problem to the caller
	// either as a zero bytes read or as throwing
	if(_buffer_size >= n_bytes)
	{
		// copy the buffers
		::memcpy(&buf[0], &_buffer[0], n_bytes);
		
		// reorganise the internal buffer
		_buffer_size -= n_bytes;
		::memmove(&_buffer[0], &_buffer[n_bytes], _buffer_size);
		
		return n_bytes;
	}
	else
	{
		return 0;
	}
}

size_t
vl::Serial::write(std::vector<char> const &buf, size_t n_bytes)
{
	if(buf.size() == 0)
	{ throw(std::string("ERROR : in Serial::write buffer can't be zero length.")); }

	size_t size = buf.size() < n_bytes ? buf.size() : n_bytes;

	write(&buf[0], size);
}

size_t
vl::Serial::write(std::string const &buf)
{
	return write(buf.c_str(), buf.size());
}

size_t
vl::Serial::write(std::vector<int> const &buf, size_t n_bytes)
{
	size_t size = buf.size() < n_bytes ? buf.size() : n_bytes;
	std::vector<char> b(size);
	for(size_t i = 0; i < size; ++i)
	{
		b.at(i) = (char)buf.at(i);
	}

	return write(&b[0], size);
}

void
vl::Serial::write(int val)
{
	size_t size = 1;
	std::vector<char> b(size);
	b.at(0) = (char)val;

	write(&b[0], size);
}

size_t
vl::Serial::write(char const *buf, size_t n_bytes)
{
	DWORD dwBytesWrite = 0;
	if(!WriteFile(_hSerial, buf, n_bytes, &dwBytesWrite, NULL))
	{
		//error occurred. Report to user.
		throw(std::string("ERROR : in Serial::write"));
	}

	return dwBytesWrite;
}

void
vl::Serial::set_read_timeout(uint32_t total_ms, uint32_t per_byte_ms)
{
	_read_total_timeout = total_ms;
	_read_per_byte_timeout = per_byte_ms;

	if(_hSerial)
	{ _set_os_read_timeout(_read_total_timeout, _read_per_byte_timeout); }
}

void
vl::Serial::set_write_timeout(uint32_t total_ms)
{
	if(!_hSerial)
	{ return; }

	COMMTIMEOUTS timeouts={0};
	
	GetCommTimeouts(_hSerial, &timeouts);

	timeouts.WriteTotalTimeoutConstant=total_ms;
	timeouts.WriteTotalTimeoutMultiplier=0;

	if(!SetCommTimeouts(_hSerial, &timeouts))
	{
		//error occureed. Inform user
		std::string err("Serial port : " + _name + " couldn't set Timeouts.");
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(err));
	}
}

size_t
vl::Serial::available(void)
{
	_copy_os_buffer();
	return _buffer_size != 0;
}

void
vl::Serial::_copy_os_buffer(void)
{
	_set_os_nonblocking_read();

	DWORD dwBytesRead = 0;
	if(!ReadFile(_hSerial, &_buffer[_buffer_size], _buffer.size()-_buffer_size, &dwBytesRead, NULL))
	{
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Win32 API : Serial ReadFile"));
	}
	_buffer_size += dwBytesRead;

	_set_os_read_timeout(_read_total_timeout, _read_per_byte_timeout);
}

void
vl::Serial::_copy_os_buffer(size_t bytes)
{
	assert(_buffer.size()-_buffer_size > bytes);

	DWORD dwBytesRead = 0;
	if(!ReadFile(_hSerial, &_buffer[_buffer_size], bytes, &dwBytesRead, NULL))
	{
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Win32 API : Serial ReadFile"));
	}

	_buffer_size += dwBytesRead;
}

void
vl::Serial::_set_os_read_timeout(uint32_t total_ms, uint32_t per_byte_ms)
{
	// @todo test with very low timelimit or zero if possible
	COMMTIMEOUTS timeouts={0};
	
	GetCommTimeouts(_hSerial, &timeouts);
	
	// Time limit between two bytes in milliseconds
	timeouts.ReadIntervalTimeout=per_byte_ms;
	// Total time limit for the whole operation, calculated with multiplier * bytes
	timeouts.ReadTotalTimeoutMultiplier=per_byte_ms;
	// Constant total timelimit for the whole read operation
	timeouts.ReadTotalTimeoutConstant=total_ms;
	
	if(!SetCommTimeouts(_hSerial, &timeouts))
	{
		//error occureed. Inform user
		std::string err("Serial port : " + _name + " couldn't set Timeouts.");
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(err));
	}
}

void
vl::Serial::_set_os_nonblocking_read(void)
{
	COMMTIMEOUTS timeouts={0};
	timeouts.ReadIntervalTimeout=MAXDWORD;
	timeouts.ReadTotalTimeoutMultiplier=0;
	timeouts.ReadTotalTimeoutConstant=0;

	if(!SetCommTimeouts(_hSerial, &timeouts))
	{
		//error occureed. Inform user
		std::string err("Serial port : " + _name + " couldn't set Timeouts.");
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(err));
	}
}
