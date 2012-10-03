/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-03
 *	@file base/time.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

/**
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
 */

#ifndef HYDRA_TIME_HPP
#define HYDRA_TIME_HPP

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
	
	time(int s);

	time(double s);

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

private :
	void _check_usecs(void);
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

}	// namespace vl

#endif // HYDRA_TIMER_HPP
