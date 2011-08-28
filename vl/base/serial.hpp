/**	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-07
 *	@file base/serial.hpp
 *
 *	This file is part of Hydra a VR game engine.
 *
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
	size_t read(std::vector<char> &buf, size_t n_bytes);

	/// @brief write the vector of bytes to the serial port
	/// blocking
	size_t write(std::vector<char> const &buf, size_t n_bytes);

	/// @brief set timeout parameters, these only work for opened serial device
	void set_read_timeout(uint32_t total_ms, uint32_t per_byte_ms);

	void set_write_timeout(uint32_t total_ms);

private :
#ifdef _WIN32
	HANDLE _hSerial;
#endif
	std::string _name;
};

}	// namespace vl

#endif	// HYDRA_BASE_SERIAL_HPP
