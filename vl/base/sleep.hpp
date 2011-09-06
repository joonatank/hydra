/**	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-07
 *	@file base/sleep.hpp
 *
 */

#ifndef HYDRA_BASE_SLEEP_HPP
#define HYDRA_BASE_SLEEP_HPP

#include <stdint.h>

#ifdef VL_WIN32
#include <windows.h>
#else
#include <time.h>
#endif

#include "base/timer.hpp"

namespace vl
{

inline void msleep(uint32_t milliseconds)
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

inline void sleep(vl::time const &t)
{
	double ms = ((double)t)*1e3;
	msleep((uint32_t)ms);
}

}	// namespace vl

#endif	// HYDRA_BASE_SLEEP_HPP
