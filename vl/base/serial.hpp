/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-07
 *	@file base/serial.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

/**
 *	Low level serial device reader.
 *	@todo add Linux implementation
 */

#ifndef HYDRA_BASE_SERIAL_HPP
#define HYDRA_BASE_SERIAL_HPP

#ifdef _WIN32
#include <Windows.h>
#else
// Abort because this needs to be fixed
#error serial.hpp is only implemented for Windows.
#endif

// Necessary for uint32_t
#include <stdint.h>
// Necessary for device name
#include <string>
// Necessary for data buffer
#include <vector>

namespace vl
{
class Serial
{
public :
	Serial(std::string const &dev_name = std::string(), uint32_t baud_rate = 19200);

	void open(std::string const &dev_name, uint32_t baud_rate = 19200);

	~Serial(void);

	void close(void);

	/// @brief fill the vector with the data in the serial port
	/// blocking
	/// @param buf buffer where the data is to be copied
	/// @param n_bytes number of bytes to read
	/// @param blocking wheather to block or return immediately
	/// @return number of bytes read from the serial
	/// Return value will always be either 0 or n_bytes, 0 when there wasn't
	/// enough data in the buffers and n_bytes if there was.
	/// If this is non-blocking this function will always return after copying os buffer
	size_t read(std::vector<char> &buf, size_t n_bytes, bool blocking = true);

	/// @brief write the vector of bytes to the serial port
	/// blocking
	size_t write(std::vector<char> const &buf, size_t n_bytes);

	/// @brief set timeout parameters, these only work for opened serial device
	void set_read_timeout(uint32_t total_ms, uint32_t per_byte_ms);

	void set_write_timeout(uint32_t total_ms);

	/// @brief get the number of available bytes for read
	/// Does a copy from OS buffer to custom buffer
	size_t available(void);

private :
	/// @brief non blocking copy of the os buffer
	void _copy_os_buffer(void);

	/// @brief blocking copy till bytes are read from the buffer
	void _copy_os_buffer(size_t bytes);

	void _set_os_read_timeout(uint32_t total_ms, uint32_t per_byte_ms);

	void _set_os_nonblocking_read(void);

#ifdef _WIN32
	HANDLE _hSerial;
#endif
	std::string _name;

	std::vector<char> _buffer;
	size_t _buffer_size;

	uint32_t _read_total_timeout;
	uint32_t _read_per_byte_timeout;
};

}	// namespace vl

#endif	// HYDRA_BASE_SERIAL_HPP
