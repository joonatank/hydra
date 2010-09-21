
#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>

#include "init_data.hpp"

eqOgre::InitData::InitData( void )
	: _xml_data(0)
{
}

eqOgre::InitData::~InitData( void )
{
	delete [] _xml_data;
}

eqOgre::InitData::InitData( eqOgre::InitData const &a )
	: _xml_data(0)
{
	setXMLdata( a._xml_data );
	_settings = a._settings;
	_name = a._name;
}

eqOgre::InitData &
eqOgre::InitData::operator=( eqOgre::InitData const &a )
{

	setXMLdata( a._xml_data );
	_settings = a._settings;
	_name = a._name;

	return *this;
}

void 
eqOgre::InitData::setXMLdata( std::string const &data )
{
	setXMLdata( data.c_str() );
	/*
	delete [] _xml_data;
	_length = xml_data.size();
	_xml_data = new char[_length];
	::memcpy( _xml_data, xml_data.c_str(), _length );
	*/
}

void 
eqOgre::InitData::setXMLdata( char const *data )
{
	delete [] _xml_data;
	_xml_data = 0;
	// Copy the data if there is some
	if( data )
	{
		size_t length = ::strlen( data );
		_xml_data = new char[length+1];
		::memcpy( _xml_data, data, length+1 );
	}
}

vl::SettingsRefPtr 
eqOgre::InitData::getSettings( void )
{
	if( !_settings && _xml_data )
	{
		_settings.reset( new vl::Settings( _name ) );
		vl::SettingsSerializer ser( _settings );
		ser.readData( _xml_data );
	}

	return _settings;
}

// ------ Protected --------
void 
eqOgre::InitData::getInstanceData( eq::net::DataOStream& os )
{
	uint32_t length = ::strlen(_xml_data);
	os << length;
	
	for( uint32_t i = 0; i < length; ++i )
	{
		os << _xml_data[i];
	}
}

void 
eqOgre::InitData::applyInstanceData( eq::net::DataIStream& is )
{
	uint32_t length;
	is >> length;
	
	delete [] _xml_data;
	_xml_data = new char[length];
	for( size_t i = 0; i < length; ++i )
	{
		is >> _xml_data[i];
	}
}