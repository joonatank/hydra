/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-03
 *	@file timer.cpp
 *
 */

/// Interface
#include "timer.hpp"

// Necessary for debug assertions
#include <assert.h>

#ifdef _WIN32
// Necessary for performance counters
#include <Windows.h>
#else
// Necessary for unix clock
#include <time.h>
#endif

#include "base/exceptions.hpp"

/// Should not be available anywhere else as this is operating system specific
namespace
{
#ifdef _WIN32
vl::time convert_large_integer(LARGE_INTEGER const &ticks, LARGE_INTEGER const &ticks_per_s)
{
	double ticks_per_usec = ((double)ticks_per_s.QuadPart)/1e6;
	uint32_t secs = (uint32_t)(ticks.QuadPart/ticks_per_s.QuadPart);
	long long ticks_left = ticks.QuadPart%ticks_per_s.QuadPart;
	uint32_t usecs = (uint32_t)((double)ticks_left)/ticks_per_usec;
	return vl::time(secs, usecs);
}
#endif

}	// unamed namespace


/// ---------------------------- Globals ---------------------------------------
double
vl::get_system_time_accuracy(void)
{
#ifdef _WIN32
	LARGE_INTEGER tps;
	QueryPerformanceFrequency(&tps);
	return double(1e6)/double(tps.QuadPart);
#else
	timespec ts;
	if( 0 != ::clock_getres(CLOCK_MONOTONIC, &ts) )
	{
		/// @todo replace with boost system errors
		std::string desc("Failed to get reolution for Monotonic clock.");
		BOOST_THROW_EXCEPTION( vl::exception() << vl::desc(desc) );
	}
	return ((double)ts.tv_sec)*1e6+((double)ts.tv_nsec)/1e3;
#endif
}

vl::time
vl::get_system_time(void)
{
#ifdef _WIN32
	// get the high resolution counter's accuracy
	LARGE_INTEGER tps;
	QueryPerformanceFrequency(&tps);
	// what time is it?
	LARGE_INTEGER ticks;
	QueryPerformanceCounter(&ticks);
	return convert_large_integer(ticks, tps);
#else
	timespec ts;
	if( 0 != ::clock_gettime(CLOCK_MONOTONIC, &ts) )
	{
		/// @todo replace with boost system errors
		std::string desc("Failed to get time from Monotonic clock.");
		BOOST_THROW_EXCEPTION( vl::exception() << vl::desc(desc) );
	}
	return time(ts.tv_sec, ts.tv_nsec/1000);
#endif
}

std::ostream &
vl::operator<<(std::ostream& os, vl::time const &t)
{
	uint32_t usec = t.usec;
	if( t.sec > 0 )
		os << t.sec << "s ";
	if( usec >= 1e3 )
	{
		os << usec/1000 << "ms ";
		usec = usec%1000;
	}
	os << usec << "us";

	return os;
}

/// ---------------------------- Time ------------------------------------------
vl::time::time(uint32_t s, uint32_t us)
	: sec(s), usec(us)
{
	/// @todo replace with throwing
	/// disallow setting a second long usec part
	assert( us < 1e6 );
}

vl::time::operator double() const
{
	return sec + (usec/1e6);
}

vl::time &
vl::time::operator-=(vl::time const &t)
{
	if( usec < t.usec )
	{
		usec += 1e6;
		--sec;
	}
	usec -= t.usec;

	// Debug checking, we can not store negative time
	assert(sec >= t.sec);
	sec -= t.sec;

	return *this;
}

vl::time &
vl::time::operator+=(vl::time const &t)
{
	sec += t.sec;
	usec += t.usec;
	if( usec >= 1e6 )
	{
		sec += usec/(uint32_t)1e6;
		usec = usec%(uint32_t)1e6;
	}
	return *this;
}

vl::time &
vl::time::operator*=(double n)
{
	// TODO should move usecs to secs if they qualify
	sec *= n;
	usec *= n;

	return *this;
}

vl::time &
vl::time::operator/=(double n)
{
	sec /= n;
	usec /= n;

	return *this;
}

vl::time &
vl::time::operator*=(long unsigned int n)
{
	// TODO should move usecs to secs if they qualify
	sec *= n;
	usec *= n;

	return *this;
}

vl::time &
vl::time::operator/=(long unsigned int n)
{
	sec /= n;
	usec /= n;

	return *this;
}

vl::time &
vl::time::operator*=(size_t n)
{
	// TODO should move usecs to secs if they qualify
	sec *= n;
	usec *= n;

	return *this;
}

vl::time &
vl::time::operator/=(size_t n)
{
	sec /= n;
	usec /= n;

	return *this;
}
