/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2010-12
 *	@file stats.hpp
 */

#ifndef VL_STATS_HPP
#define VL_STATS_HPP

#include <vector>
#include <iostream>
#include <string>

#include "base/timer.hpp"

namespace vl
{

class StatElem
{
public :
	StatElem( std::string const &text );

	void addTime(vl::time const &t);

	double getAvarage(void) const;

	void updateAvarage(void);

	void clear(void);

	bool isValid(void) const
	{ return !_times.empty(); }

	std::string const &getText(void) const
	{ return _text; }

private :
	std::string _text;

	std::vector<double> _times;

	double _avarage;
};

std::ostream & operator<<(std::ostream &os, StatElem const &elem);

/**	@class Stats
 *	@brief class used for logging and printing statistics
 *
 *	@todo Should be separated to normal statistics and frame statistics
 *	normal statistics are used for loading and similar tasks and 
 *	frame statistics are used for statistics gathered every frame and avaraged
 *	over time.
 */
class Stats
{
public :
	Stats( void );

	~Stats( void );

	/**	@brief Log time used for rendering a frame to the stats
	 *	@param time time in milliseconds
	 */
	void logRenderingTime(vl::time const &t);

	void logLocalRenderingTime(vl::time const &t);

	void logLocalSwapTime(vl::time const &t);

	void logLocalCaptureTime(vl::time const &t);
	
	void logWaitDrawDoneTime(vl::time const &t);

	void logWaitDrawTime(vl::time const &t);

	void logWaitUpdateTime(vl::time const &t);

	/**	@brief Log time used for processing events to the stats
	 *	@param time time in milliseconds
	 */
	void logEventProcessingTime(vl::time const &t);

	/**	@brief Log time used for creating update to the stats
	 *	@param time time in milliseconds
	 */
	void logUpdateTime(vl::time const &t);

	/**	@brief Log time used for stepping the simulation to the stats
	 *	@param time time in milliseconds
	 */
	void logStepTime(vl::time const &t);

	/**	@brief Log time used for stepping the simulation to the stats
	 *	@param time time in milliseconds
	 */
	void logInitTime(vl::time const &t);

	/**	@brief Update the avarages and clear the current gathered values
	 */
	void update(void);

	friend std::ostream &operator<<(std::ostream &os, Stats const &stats);

private :
	StatElem _rendering_time;
	StatElem _local_rendering_time;
	StatElem _local_swap_time;
	StatElem _local_capture_time;

	StatElem _event_time;
	StatElem _update_time;
	StatElem _step_time;
	StatElem _wait_update_time;
	StatElem _wait_draw_time;
	StatElem _wait_draw_done_time;

	double _init_time;

};	// class Stats

std::ostream &operator<<(std::ostream &os, Stats const &stats);

}	// namespace vl

#endif	// VL_STATS_HPP
