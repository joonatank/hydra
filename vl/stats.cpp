/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2010-12
 *	@file stats.cpp
 */

#include "stats.hpp"

#include <iostream>

vl::Stats::Stats( void )
	: _init_time(0)
{}

vl::Stats::~Stats( void )
{}

void
vl::Stats::logRenderingTime( double time )
{
	_rendering_times.push_back(time);
}

void
vl::Stats::logEventProcessingTime( double time )
{
	_event_times.push_back(time);
}

void
vl::Stats::logFrameProcessingTime( double time )
{
	_frame_times.push_back(time);
}

void
vl::Stats::logUpdateTime( double time )
{
	_update_times.push_back(time);
}

void
vl::Stats::logStepTime( double time )
{
	_step_times.push_back(time);
}

void 
vl::Stats::logInitTime( double time )
{
	_init_time = time;
}

void 
vl::Stats::clear( void )
{
	_rendering_times.clear();
	_event_times.clear();
	_frame_times.clear();
	_update_times.clear();
	_step_times.clear();
	_init_time = 0;
}

void 
vl::Stats::print( void )
{
	double avg = _calculate_avarage( _rendering_times );
	double total = avg;
	std::cout << "Rendering time avarage = " << avg << "ms." << std::endl;

	avg = _calculate_avarage( _event_times );
	total += avg;
	std::cout << "Event processing time avarage = " << avg << "ms." 
		<< std::endl;
	
	avg = _calculate_avarage( _frame_times );
	total += avg;
	std::cout << "Frame Processing time avarage = " << avg << "ms." 
		<< std::endl;
	
	avg = _calculate_avarage( _update_times );
	total += avg;
	std::cout << "Simulation update time avarage = " << avg << "ms." 
		<< std::endl;

	avg = _calculate_avarage( _step_times );
	total += avg;
	std::cout << "Simulation step time avarage = " << avg << "ms." 
		<< std::endl;

	std::cout << "Total frame processing time = " << total << "ms." 
		<< std::endl;

	std::cout << "Init time = " << _init_time << "ms." << std::endl;
}

double 
vl::Stats::_calculate_avarage( std::vector<double> const &v )
{
	double avg = 0;
	for( size_t i = 0; i < v.size(); ++i )
	{
		avg += v.at(i);
	}

	return avg/v.size();
}
