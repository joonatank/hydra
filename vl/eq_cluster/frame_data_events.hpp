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
	ReloadScene( void )
	{
	}

	void setFrameData( FrameData *data )
	{
		_frame_data = data;
	}

	FrameData *getFrameData( void )
	{ return _frame_data; }

	virtual void execute( void )
	{
		if( !_frame_data )
		{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

		_frame_data->updateSceneVersion();
	}

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }

	static const std::string TYPENAME;

private :
	FrameData *_frame_data;
};

}
#endif