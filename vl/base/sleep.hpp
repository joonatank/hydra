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

inline void msleep( uint32_t milliseconds )
{
#ifdef _WIN32
	// Easy to disable zero millisecond sleep on Windows which at least is a problem
	// for Windows XP
	#ifndef WIN_ZERO_SLEEP
	if(milliseconds == 0)
	{ return; }
	#endif
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
