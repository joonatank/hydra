#ifndef VL_BASE_HELPERS_HPP
#define VL_BASE_HELPERS_HPP

/// C headers
// Necessary for getpid()
#include <stdint.h>

#ifdef VL_WIN32
#include <Windows.h>
#endif

namespace vl
{

inline uint32_t getPid( void )
{
#ifdef VL_WIN32
	return GetCurrentProcessId();
#else
	return ::getpid();
#endif
}

}

#endif
