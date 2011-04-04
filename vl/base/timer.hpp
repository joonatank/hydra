/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-03
 *	@file timer.hpp
 *
 *	Linux and Windows implementation is provided.
 *
 *	Time functions and timers provide microsecond resolution on most current machines
 *	this can be checked using get_system_time_accuracy function.
 *
 *	time class is provided for ease of usage, this follows the same design as
 *	BSD timeval/timespec, but provides operators in c++ fashion.
 *
 *	portable system functions for performance timing are provided
 *
 *	Timer class for counting elapsed time is provided.
 *
 *	@todo add an interrupt based timed trigger.
 *	Would have a callback functor attached to it.
 *	These would be used for the messaging system in UDP server/client
 *	for resending messages and similar.
 *
 *	Timers do not use the wall clock.
 *	This is because the following pseudo code should work fine.
 *		timer t
 *		setSystemClock(current_time - 20) # secs doesn't really matter
 *		assert( t.getTime() > 0 )
 *	Wall clocks or system clocks fail miserably because their time can be
 *	modified. Timers should never have their time moved backwards.
 *
 *
 */

#ifndef HYDRA_TIMER_HPP
#define HYDRA_TIMER_HPP

// Necessary for uint32_t
#include <stdint.h>
// Necessary for stream operators
#include <iostream>

// still using the old namespace
namespace vl
{

/// Timeval structure with functionality
/// @todo should this be changed to a class (private data)
/// would allow us to change to from microsecond to nanosecond
struct time
{
	time(uint32_t s = 0, uint32_t us = 0);
	uint32_t sec;
	uint32_t usec;

	/// conversion to seconds
	operator double() const;

	/// Arithmetic
	time &operator-=(time const &t);

	time &operator+=(time const &t);

	time &operator/=(double n);

	time &operator*=(double n);

	time &operator/=(size_t n);

	time &operator*=(size_t n);

	time &operator/=(unsigned long n);

	time &operator*=(unsigned long n);
};

/// time arithmetic operators
inline
time operator+(time const &t1, time const &t2)
{
	time tmp(t1);
	tmp += t2;
	return tmp;
}

inline
time operator-(time const &t1, time const &t2)
{
	time tmp(t1);
	tmp -= t2;
	return tmp;
}

template<typename T>
inline
time operator*(time const &t1, T n)
{
	time tmp(t1);
	tmp *= n;
	return tmp;
}

template<typename T>
inline
time operator/(time const &t1, T n)
{
	time tmp(t1);
	tmp /= n;
	return tmp;
}

/// time comparison operators
inline
bool operator==(time const &t1, time const &t2)
{
	return t1.sec == t2.sec && t1.usec == t2.usec;
}

inline
bool operator!=(time const &t1, time const &t2)
{
	return !(t1 == t2);
}

inline
bool operator<(time const &t1, time const &t2)
{
	if( t1.sec == t2.sec )
	{ return t1.usec < t2.usec; }
	else
	{ return t1.sec < t2.sec; }
}

inline
bool operator>(time const &t1, time const &t2)
{
	return t2 < t1;
}

inline
bool operator<=(time const &t1, time const &t2)
{
	return t1 < t2 || t1 == t2;
}

inline
bool operator>=(time const &t1, time const &t2)
{
	return t1 > t2 || t1 == t2;
}


/// time stream operators
std::ostream &operator<<(std::ostream &os, time const &t);

/// Returns the system time accuracy (and the accuracy of the timer) in microseconds
double get_system_time_accuracy(void);

/// Function to get the current process time using the time structure
/// accuracy is system dependent but it should be around one microsecond
/// on current machines.
time get_system_time(void);

/// @class timer used to measure timevals between creation/reset and query
class timer
{
public :
	/// @brief Construct a timer with time set to current time
	timer(void);

	/// @brief Construct a timer with initial time at some point in the history
	/// Basicly this adds an initial value for elapsed.
	/// Useful for timers that are there to restrict events occuring too fast.
	timer(time const &t);

	/// @brief reset the start time to now
	void reset(void);

	/// @brief time elapsed since last reset
	/// @return time between last call to reset and now
	time elapsed(void) const;

private:
	time _start_time;   // A point in time

};

}	// namespace vl

/// ----------------------------- Inlines --------------------------------------

/// ------------------------------ Timer ---------------------------------------
inline vl::timer::timer(void )
{ reset(); }

inline vl::timer::timer(time const &t)
{
	reset();
	_start_time -= t;
}

inline void
vl::timer::reset(void )
{ _start_time = vl::get_system_time(); }

inline vl::time
vl::timer::elapsed(void) const
{ return vl::get_system_time() - _start_time; }


#endif // HYDRA_TIMER_HPP
