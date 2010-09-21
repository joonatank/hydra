#ifndef VL_EQ_INIT_DATA_HPP
#define VL_EQ_INIT_DATA_HPP

#include <eq/net/object.h>

#include <string>

#include "settings.hpp"

namespace eqOgre
{

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

	vl::SettingsRefPtr getSettings( void );

protected:
    virtual void getInstanceData( eq::net::DataOStream& os );
    virtual void applyInstanceData( eq::net::DataIStream& is );

private :
	std::string _name;

	char *_xml_data;

	vl::SettingsRefPtr _settings;
};

}

#endif