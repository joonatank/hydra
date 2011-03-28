/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-03
 *	@file timer.hpp
 *
 *	Only Windows implementation is provided for now.
 *	@todo Linux version
 */

#ifndef HYDRA_TIMER_HPP
#define HYDRA_TIMER_HPP

//#include <time.h>
// Necessary for uint32_t
#include <stdint.h>
// Necessary for stream operators
#include <iostream>

#ifdef _WIN32
// Necessary for performance counters
#include <Windows.h>
#else
assert(false && "Timers only implemented for Windows")
#endif

// still using the old namespace
namespace vl
{

/// Timeval structure with functionality
struct time
{
	time(uint32_t s = 0, uint32_t us = 0)
		: sec(s), usec(us)
	{}

	uint32_t sec;
	uint32_t usec;

	// conversion to seconds
	operator double() const
	{
		return sec + (usec/1e6);
	}

	/// Arithmetic
	// This gives negative time, should we fix it?
	time &operator-=(time const &t)
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
		
	time &operator+=(time const &t)
	{
		sec += t.sec;
		usec += t.usec;
		return *this;
	}
	
};

/// time arithmetic operators
time operator+(time const &t1, time const &t2)
{
	time tmp(t1);
	tmp += t2;
	return tmp;
}

time operator-(time const &t1, time const &t2)
{
	time tmp(t1);
	tmp -= t2;
	return tmp;
}

/// time comparison operators
bool operator==(time const &t1, time const &t2)
{
	return t1.sec == t2.sec && t1.usec == t2.usec;
}

bool operator!=(time const &t1, time const &t2)
{
	return !(t1 == t2);
}

bool operator<(time const &t1, time const &t2)
{
	if( t1.sec == t2.sec )
	{ return t1.usec < t2.usec; }
	else
	{ return t1.sec < t2.sec; }
}

bool operator>(time const &t1, time const &t2)
{
	return t2 < t1;
}

bool operator<=(time const &t1, time const &t2)
{
	return t1 < t2 || t1 == t2;
}

bool operator>=(time const &t1, time const &t2)
{
	return t1 > t2 || t1 == t2;
}


/// time stream operators
std::ostream &operator<<(std::ostream &os, const time &t)
{
	uint32_t usec = t.usec;
	if( t.sec > 0 )
		os << t.sec << "s ";
	if( usec >= 1e3 )
	{
		os << usec/1000 << "ms ";
		usec = usec%1000;
	}
	os << usec << "us ";
	
	return os;
}

/// Returns the system time accuracy (and the accuracy of the timer) in microseconds
inline
double get_system_time_accuracy(void)
{
	LARGE_INTEGER tps;
	QueryPerformanceFrequency(&tps);
	return double(1e6)/double(tps.QuadPart);
}

inline
time convert_large_integer(LARGE_INTEGER const &ticks, LARGE_INTEGER const &ticks_per_s)
{
	double ticks_per_usec = ticks_per_s.QuadPart/1e6;
	uint32_t secs = (uint32_t)ticks.QuadPart/ticks_per_s.QuadPart;
	uint32_t usecs = (uint32_t)((ticks.QuadPart%ticks_per_s.QuadPart)/ticks_per_usec);
	return time(secs, usecs);
}

/// Function to get the current process time using the time structure
/// accuracy is system dependent but it should be around one microsecond
/// on current machines.
inline
time get_system_time(void)
{
	// get the high resolution counter's accuracy
	LARGE_INTEGER tps;
	QueryPerformanceFrequency(&tps);
	// what time is it?
	LARGE_INTEGER ticks;
	QueryPerformanceCounter(&ticks);
	return convert_large_integer(ticks, tps);
}

/// Class to measure timevals between creation/reset and query
class timer
{
public :
	timer(void)
	{
		reset();
	}

	void reset(void)
	{
		_start_time = vl::get_system_time();
	}

	/// The internal structure, microsecond accuracy
	time getTime(void)
	{
		return vl::get_system_time() - _start_time;
	}

private:
	time _start_time;   // A point in time
	
};

}	// namespace vl

#endif // HYDRA_TIMER_HPP
