#ifndef EQ_OGRE_SETTINGS_HPP
#define EQ_OGRE_SETTINGS_HPP

#include <eq/net/object.h>

#include "../settings.hpp"

namespace eqOgre
{
	
class Settings : public vl::Settings, public eq::net::Object
{
public :
	Settings( std::string const &name  );

	virtual ~Settings( void )
	{}

	uint32_t getFrameDataID( void ) const
	{ return _frame_data_id; }

	void setFrameDataID( uint32_t const id )
	{ _frame_data_id = id; }

protected :
	virtual void getInstanceData( eq::net::DataOStream& os );
	virtual void applyInstanceData( eq::net::DataIStream& is );

	uint32_t _frame_data_id;
};

}	// namespace eqOgre

#endif //EQ_OGRE_SETTINGS_HPP