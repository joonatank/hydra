#ifndef EQ_OGRE_FRAME_DATA_EVENTS_HPP
#define EQ_OGRE_FRAME_DATA_EVENTS_HPP

#include "event.hpp"
#include "frame_data.hpp"
#include <base/exceptions.hpp>

namespace eqOgre
{

class ReloadScene : public Operation
{
public :
	ReloadScene( FrameData *data )
		: _frame_data(data)
	{
		if( !_frame_data )
		{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }
	}

	virtual void operator()( void )
	{
		_frame_data->updateSceneVersion();
	}

private :
	FrameData *_frame_data;
};

}
#endif