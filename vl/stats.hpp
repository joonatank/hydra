/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2010-12
 *	@file stats.hpp
 */

#ifndef VL_STATS_HPP
#define VL_STATS_HPP

#include <vector>

namespace vl
{

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
	void logRenderingTime( double time );

	/**	@brief Log time used for processing events to the stats
	 *	@param time time in milliseconds
	 */
	void logEventProcessingTime( double time );

	/**	@brief Log time used for processing frame events to the stats
	 *	@param time time in milliseconds
	 */
	void logFrameProcessingTime( double time );

	/**	@brief Log time used for creating update to the stats
	 *	@param time time in milliseconds
	 */
	void logUpdateTime( double time );

	/**	@brief Log time used for stepping the simulation to the stats
	 *	@param time time in milliseconds
	 */
	void logStepTime( double time );

	/**	@brief Log time used for stepping the simulation to the stats
	 *	@param time time in milliseconds
	 */
	void logInitTime( double time );

	/**	@brief Clear the statistics gathered
	 */
	void clear( void );

	void print( void );

private :
	double _calculate_avarage( std::vector<double> const &v );

	std::vector<double> _rendering_times;
	std::vector<double> _event_times;
	std::vector<double> _frame_times;
	std::vector<double> _update_times;
	std::vector<double> _step_times;
	double _init_time;

};	// class Stats

}	// namespace vl

#endif	// VL_STATS_HPP