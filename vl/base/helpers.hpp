#ifndef VL_BASE_HELPERS_HPP
#define VL_BASE_HELPERS_HPP

#include <stdint.h>

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
