
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

void 
eqOgre::InitData::setXMLdata( std::string const &xml_data )
{
	delete [] _xml_data;
	_xml_data = new char[xml_data.size()];
	::memcpy( _xml_data, xml_data.c_str(), xml_data.size() );
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