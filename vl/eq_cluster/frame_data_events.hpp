#ifndef EQ_OGRE_FRAME_DATA_EVENTS_HPP
#define EQ_OGRE_FRAME_DATA_EVENTS_HPP

#include "event.hpp"
#include "frame_data.hpp"
#include <base/exceptions.hpp>

namespace eqOgre
{

// TODO this should have a time limit also
class ReloadScene : public Operation
{
public :
	ReloadScene( FrameData *data, double timeLimit )
		: _frame_data(data), _last_time( ::clock() ), _time_limit(timeLimit)
	{
		if( !_frame_data )
		{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }
	}

	virtual void operator()( void )
	{
		// TODO the clock needs to be moved to the Trigger
		clock_t time = ::clock();
		std::cerr << "ReloadScene::operator()" << std::endl;

		// We need to wait _time_limit secs before issuing the command again
		if( ( (double)(time - _last_time) )/CLOCKS_PER_SEC > _time_limit )
		{
			_frame_data->updateSceneVersion();
			_last_time = time;
		}
	}

private :
	FrameData *_frame_data;
	clock_t _last_time;
	double _time_limit;
};

}
#endif