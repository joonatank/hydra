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

	size_t size( void ) const
	{ return _length; }

	vl::SettingsRefPtr getSettings( void );

protected:
    virtual void getInstanceData( eq::net::DataOStream& os );
    virtual void applyInstanceData( eq::net::DataIStream& is );

private :
	char *_xml_data;
	size_t _length;
	vl::SettingsRefPtr _settings;
};

}

#endif