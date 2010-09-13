#ifndef VL_EQ_INIT_DATA_HPP
#define VL_EQ_INIT_DATA_HPP

#include <eq/net/object.h>

#include <string>

namespace eqOgre
{

class InitData : public eq::net::Object
{
public :
	InitData( void );
	~InitData( void );

	void setXMLdata( std::string const &xml_data );
	char *getXMLdata( void )
	{ return _xml_data; }

protected:
    virtual void getInstanceData( eq::net::DataOStream& os );
    virtual void applyInstanceData( eq::net::DataIStream& is );

private :
	char *_xml_data;
};

}

#endif