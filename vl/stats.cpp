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
	, _avarage(0)
{}

void 
vl::StatElem::addTime(vl::time const &t)
{
	_times.push_back(((double)t)*1e3);
}

double 
vl::StatElem::getAvarage(void) const
{
	return _avarage;
}

void 
vl::StatElem::updateAvarage(void)
{
	_avarage = calculate_avarage(_times);
	clear();
}

void 
vl::StatElem::clear(void)
{
	_times.clear();
}

vl::Stats::Stats( void )
	: _init_time(0)
	, _rendering_time("Rendering")
	, _local_rendering_time("Local Rendering")
	, _local_swap_time("Local Swap")
	, _local_capture_time("Local capture")
	, _event_time("Event processing")
	, _update_time("Update")
	, _step_time("Step")
	, _wait_draw_done_time("Wait draw done")
	, _wait_draw_time("Wait draw")
	, _wait_update_time("Wait update")
{}

vl::Stats::~Stats( void )
{}

void
vl::Stats::logRenderingTime(vl::time const &t)
{
	_rendering_time.addTime(t);
}

void
vl::Stats::logLocalRenderingTime(vl::time const &t)
{
	_local_rendering_time.addTime(t);
}

void
vl::Stats::logLocalSwapTime(const vl::time& t)
{
	_local_swap_time.addTime(t);
}

void
vl::Stats::logLocalCaptureTime(vl::time const &t)
{
	_local_capture_time.addTime(t);
}

void
vl::Stats::logWaitDrawDoneTime(vl::time const &t)
{
	_wait_draw_done_time.addTime(t);
}

void
vl::Stats::logWaitDrawTime(vl::time const &t)
{
	_wait_draw_time.addTime(t);
}

void
vl::Stats::logWaitUpdateTime(vl::time const &t)
{
	_wait_update_time.addTime(t);
}

void
vl::Stats::logEventProcessingTime(vl::time const &t)
{
	_event_time.addTime(t);
}

void
vl::Stats::logUpdateTime(vl::time const &t)
{
	_update_time.addTime(t);
}

void
vl::Stats::logStepTime(vl::time const &t)
{
	_step_time.addTime(t);
}

void 
vl::Stats::logInitTime(vl::time const &t)
{
	_init_time = ((double)t)*1e3;
}

void 
vl::Stats::update(void)
{
	_rendering_time.updateAvarage();
	_local_rendering_time.updateAvarage();
	_local_capture_time.updateAvarage();
	_local_swap_time.updateAvarage();

	_event_time.updateAvarage();
	_update_time.updateAvarage();
	_step_time.updateAvarage();
	_wait_update_time.updateAvarage();
	_wait_draw_done_time.updateAvarage();
	_wait_draw_time.updateAvarage();
}

std::ostream &
vl::operator<<(std::ostream &os, vl::Stats const &stats)
{
	double total = stats._rendering_time.getAvarage() + stats._event_time.getAvarage()
		+ stats._update_time.getAvarage() + stats._step_time.getAvarage();

	os << stats._rendering_time << std::endl;
	os << stats._local_rendering_time << std::endl;
	os << stats._local_swap_time << std::endl;
	os << stats._local_capture_time << std::endl;
	os << stats._event_time << std::endl;
	os << stats._update_time << std::endl;
	os << stats._step_time << std::endl;
	os << stats._wait_update_time << std::endl;
	os << stats._wait_draw_done_time << std::endl;
	os << stats._wait_draw_time << std::endl;

	os << "Total frame processing time = " << total << "ms." 
		<< std::endl;

	if( stats._init_time != 0 )
	{ os << "Init time = " << stats._init_time << "ms." << std::endl; }

	return os;
}
