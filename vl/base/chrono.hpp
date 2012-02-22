/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011/10 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-03
 *	@file base/chrono.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

/*
 *	Timer class for counting elapsed time is provided.
 *
 *	@todo add an interrupt based timed trigger.
 *	Would have a callback functor attached to it.
 *	These would be used for the messaging system in UDP server/client
 *	for resending messages and similar.
 *
 *	@todo add chrono that counts-down from some specific moment
 *	or toward a specific moment and invokes a callback when reached.
 *	Looping and non-looping would be possible.
 *
 *	Timers do not use the wall clock.
 *	This is because the following pseudo code should work fine.
 *		timer t
 *		setSystemClock(current_time - 20) # secs doesn't really matter
 *		assert( t.getTime() > 0 )
 *	Wall clocks or system clocks fail miserably because their time can be
 *	modified. Timers should never have their time moved backwards.
 *
 */

#ifndef HYDRA_BASE_CHRONO_HPP
#define HYDRA_BASE_CHRONO_HPP

#include "time.hpp"


namespace vl
{

/// @class timer used to measure timevals between creation/reset and query
/// @todo rename to chrono as it's what this does it's a clock not a timer
class chrono
{
public :
	/// @brief Construct a timer with time set to current time
	chrono(void);

	/// @brief Construct a timer with initial time at some point in the history
	/// Basicly this adds an initial value for elapsed.
	/// Useful for timers that are there to restrict events occuring too fast.
	chrono(time const &t);

	/// @brief reset the start time to now
	void reset(void);

	/// @brief time elapsed since last reset
	/// @return time between last call to reset and now
	time elapsed(void) const;

private:
	time _start_time;   // A point in time

};

class stop_chrono
{
public :
	/// Construct a timer starting from now and running
	stop_chrono(void);

	/// @brief resume counting
	void resume(void);

	/// @brief stop the counting
	/// The time between call to a stop and a subsequent call to resume will
	/// not be present when elapsed is called.
	void stop(void);

	/// @brief reset the current time and put the clock running
	void reset(void);

	/// @brief time elapsed since last reset accounting for the stopping
	time elapsed(void) const;

private :
	time _elapsed;
	time _last_time;
	bool _stopped;
};

}	// namespace vl

#endif HYDRA_BASE_CHRONO_HPP
