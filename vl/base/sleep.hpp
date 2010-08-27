#ifndef VL_BASE_SLEEP_HPP
#define VL_BASE_SLEEP_HPP

#include <stdint.h>

#ifdef VL_WIN32
#include <windows.h>
#else
#include <time.h>
#endif

namespace vl
{

void usleep( uint32_t milliseconds )
{
#ifdef VL_WIN32
	::Sleep(milliseconds);
#else
	timespec tv;
	tv.tv_sec = 0;
	tv.tv_nsec = milliseconds * 1e3;
	::nanosleep( &tv, 0 );
#endif
}

}

#endif