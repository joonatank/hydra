/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-08
 *	@file base/system_utils.hpp
 *
 *	This file is part of Hydra VR game engine.
 */

#ifndef HYDRA_SYSTEM_UTILS_HPP
#define HYDRA_SYSTEM_UTILS_HPP

/// Necessary for uint32_t
#include <stdint.h>

#include <string>
#include <vector>


#include "exceptions.hpp"

namespace vl
{

uint32_t getPid(void);

/// @brief Create a new process
/// @param path the path to the binary to start
/// @param params what to pass to the process
/// @return PID
/// @todo should this return a process information structure similar to Win32 API instead
uint32_t create_process(std::string const &path, std::vector<std::string> const &params, bool create_new_console = false);

void kill_process(uint32_t pid);

}

#endif	// HYDRA_SYSTEM_UTILS_HPP
