#ifndef VL_EQ_INIT_DATA_HPP
#define VL_EQ_INIT_DATA_HPP

#include <eq/net/object.h>

#include <string>

#include "settings.hpp"

namespace eqOgre
{

// TODO this is problematic class and it should be replaced by distributed
// version of Settings class
// Just extend the current one
class InitData : public eq::net::Object
{
public :
	InitData( void );
	~InitData( void );
	InitData( InitData const &a );
	InitData& operator=( InitData const &a );

	void setXMLdata( std::string const &xml_data );
	void setXMLdata( char const *xml_data );

	char const *getXMLdata( void ) const
	{ return _xml_data; }

	void setName( std::string const &name )
	{ _name = name; }

	std::string const &getName( void )
	{ return _name; }

	size_t size( void ) const
	{ return ::strlen(_xml_data); }

	// TODO this is temporary function should be removed
	void setSettings( vl::SettingsRefPtr set )
	{
		_settings = set;
	}

	// TODO fix the constness of this function
	// Non const stuff should be moved to applyInstanceData
	vl::SettingsRefPtr getSettings( void );

	uint32_t getFrameDataID( void ) const
	{ return _frame_data_id; }

	void setFrameDataID( uint32_t const id )
	{ _frame_data_id = id; }

protected:
    virtual void getInstanceData( eq::net::DataOStream& os );
    virtual void applyInstanceData( eq::net::DataIStream& is );

private :
	std::string _name;

	uint32_t _frame_data_id;

	char *_xml_data;

	vl::SettingsRefPtr _settings;
};

}

#endif