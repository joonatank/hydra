#ifndef VL_BASE_SLEEP_HPP
#define VL_BASE_SLEEP_HPP

#include <stdint.h>

#ifdef VL_WIN32
#include <winsock2.h>
#include <windows.h>
#include <windef.h>
#else
#include <time.h>
#endif

namespace vl
{

void msleep( uint32_t milliseconds )
{
#ifdef VL_WIN32
	::Sleep(milliseconds);
#else
	timespec tv;
	tv.tv_sec = 0;
	tv.tv_nsec = milliseconds * 1e6;
	::nanosleep( &tv, 0 );
#endif
}

}

#endif
