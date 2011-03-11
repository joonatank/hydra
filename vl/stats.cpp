/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2010-12
 *	@file stats.cpp
 */

#include "stats.hpp"

#include <iostream>

namespace 
{

double 
calculate_avarage( std::vector<double> const &v )
{
	if( v.size() == 0 )
	{ return 0; }

	double avg = 0;
	for( size_t i = 0; i < v.size(); ++i )
	{
		avg += v.at(i);
	}

	return avg/v.size();
}

}

std::ostream &
vl::operator<<(std::ostream &os, vl::StatElem const &elem)
{
	os << elem.getText() << " avarage time = " << elem.getAvarage() << "ms.";

	return os;
}


vl::StatElem::StatElem( std::string const &text )
	: _text(text)
{}

void 
vl::StatElem::addTime(double time)
{
	_times.push_back(time);
}

double 
vl::StatElem::getAvarage(void) const
{
	return calculate_avarage(_times);
}

void 
vl::StatElem::clear(void)
{
	_times.clear();
}

vl::Stats::Stats( void )
	: _init_time(0)
	, _rendering_time("Rendering")
	, _event_time("Event processing")
	, _frame_time("Frame processing")
	, _update_time("Update")
	, _step_time("Step")
	, _wait_swap_time("Wait swap")
	, _wait_draw_time("Wait draw")
	, _wait_update_time("Wait update")
{}

vl::Stats::~Stats( void )
{}

void
vl::Stats::logRenderingTime( double time )
{
	_rendering_time.addTime(time);
}

void
vl::Stats::logWaitSwapTime( double time )
{
	_wait_swap_time.addTime(time);
}

void
vl::Stats::logWaitDrawTime( double time )
{
	_wait_draw_time.addTime(time);
}

void
vl::Stats::logWaitUpdateTime( double time )
{
	_wait_update_time.addTime(time);
}

void
vl::Stats::logEventProcessingTime( double time )
{
	_event_time.addTime(time);
}

void
vl::Stats::logFrameProcessingTime( double time )
{
	_frame_time.addTime(time);
}

void
vl::Stats::logUpdateTime( double time )
{
	_update_time.addTime(time);
}

void
vl::Stats::logStepTime( double time )
{
	_step_time.addTime(time);
}

void 
vl::Stats::logInitTime( double time )
{
	_init_time = time;
}

void 
vl::Stats::clear( void )
{
	_rendering_time.clear();
	_event_time.clear();
	_frame_time.clear();
	_update_time.clear();
	_step_time.clear();
	_init_time = 0;
}

std::ostream &
vl::operator<<(std::ostream &os, vl::Stats const &stats)
{
	double total = stats._rendering_time.getAvarage() + stats._event_time.getAvarage()
		+ stats._frame_time.getAvarage() + stats._update_time.getAvarage()
		+ stats._step_time.getAvarage();

	os << stats._rendering_time << std::endl;
	os << stats._event_time << std::endl;
	os << stats._frame_time << std::endl;
	os << stats._update_time << std::endl;
	os << stats._step_time << std::endl;
	os << stats._wait_update_time << std::endl;
	os << stats._wait_swap_time << std::endl;
	os << stats._wait_draw_time << std::endl;

	os << "Total frame processing time = " << total << "ms." 
		<< std::endl;

	if( stats._init_time != 0 )
	{ os << "Init time = " << stats._init_time << "ms." << std::endl; }

	return os;
}
