/**	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-07
 *	@file base/serial.hpp
 *
 *	This file is part of Hydra a VR game engine.
 *
 *	Low level serial device reader.
 *	@todo add Linux implementation
 */

// Interface
#include "serial.hpp"

#include <iostream>

#include "exceptions.hpp"

vl::Serial::Serial(std::string const &dev_name, uint32_t baud_rate)
	: _hSerial(0)
{
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
		if(GetLastError()==ERROR_FILE_NOT_FOUND)
		{
			//serial port does not exist. Inform user.
			// @todo should throw
			std::cerr << "ERROR : No such COM port." << std::endl;
		}
		else
		{
			//some other error occurred. Inform user.
			// @todo should throw
			std::cerr << "ERROR : Unknow error type." << std::endl;
		}
		throw std::string("error");
	}

	// Set options
	DCB dcbSerialParams = {0};
	dcbSerialParams.DCBlength=sizeof(dcbSerialParams);
	if (!GetCommState(_hSerial, &dcbSerialParams)) 
	{
		//error getting state
		std::cerr << "Serial port : " << _name
			<< " couldn't get the state." << std::endl;
		throw std::string("error");
	}

	dcbSerialParams.BaudRate = baud_rate;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity = NOPARITY;
	if(!SetCommState(_hSerial, &dcbSerialParams))
	{
		//error setting serial port state
		std::cerr << "Serial port : " << _name
			<< " couldn't set the state." << std::endl;
		throw std::string("error");
	}

	// set some default timeouts so the application will not hang
	set_read_timeout(5, 50);
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
vl::Serial::read(std::vector<char> &buf, size_t n_bytes)
{
	if(buf.size() == 0)
	{ throw(std::string("ERROR : in Serial::read buffer can't be zero length.")); }

	if(n_bytes == 0)
	{ n_bytes = buf.size(); }

	DWORD dwBytesRead = 0;
	if(!ReadFile(_hSerial, &buf[0], n_bytes, &dwBytesRead, NULL))
	{
		//error occurred. Report to user.
		throw(std::string("ERROR : in Serial::read"));
	}

	return dwBytesRead;
}

size_t
vl::Serial::write(std::vector<char> const &buf, size_t n_bytes)
{
	if(buf.size() == 0)
	{ throw(std::string("ERROR : in Serial::write buffer can't be zero length.")); }

	if(n_bytes == 0)
	{ n_bytes = buf.size(); }

	DWORD dwBytesWrite = 0;
	if(!WriteFile(_hSerial, &buf[0], n_bytes, &dwBytesWrite, NULL))
	{
		//error occurred. Report to user.
		throw(std::string("ERROR : in Serial::write"));
	}

	return dwBytesWrite;
}


void
vl::Serial::set_read_timeout(uint32_t total_ms, uint32_t per_byte_ms)
{
	if(!_hSerial)
	{ return; }

	// @todo test with very low timelimit or zero if possible
	COMMTIMEOUTS timeouts={0};
	
	GetCommTimeouts(_hSerial, &timeouts);
	
	// Time limit between two bytes in milliseconds
	timeouts.ReadIntervalTimeout=per_byte_ms;
	// Total time limit for the whole operation, calculated with multiplier * bytes
	timeouts.ReadTotalTimeoutMultiplier=0;
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
