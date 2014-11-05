/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-03
 *	@file base/chrono.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

/// Interface
#include "chrono.hpp"

/// ------------------------------ chrono ------------------------------------
vl::chrono::chrono(void )
{ reset(); }

vl::chrono::chrono(time const &t)
{
	reset();
	_start_time -= t;
}

void
vl::chrono::reset(void )
{ _start_time = vl::get_system_time(); }

vl::time
vl::chrono::elapsed(void) const
{ return vl::get_system_time() - _start_time; }

/// ----------------------------- stop_chrono --------------------------------

vl::stop_chrono::stop_chrono(void)
	: _last_time(vl::get_system_time())
	, _stopped(false)
{}

void
vl::stop_chrono::resume(void)
{
	// If the clock is already on do nothing
	if(_stopped)
	{
		_stopped = false;
		_last_time = vl::get_system_time();
	}
}

void
vl::stop_chrono::stop(void)
{
	// If the clock is already stopped do nothing
	if(!_stopped)
	{
		_stopped = true;
		_elapsed += vl::get_system_time() - _last_time;
		// No need to update last time as resume will do it
	}
}

void
vl::stop_chrono::reset(void)
{
	_elapsed = vl::time();
	_last_time = vl::get_system_time();
	_stopped = false;
}

vl::time
vl::stop_chrono::elapsed(void) const
{
	vl::time elapsed = _elapsed;
	// If the clock is not stopped we need to account for the time since resume
	if(!_stopped)
	{
		elapsed += vl::get_system_time()-_last_time;
	}

	return elapsed;
}
