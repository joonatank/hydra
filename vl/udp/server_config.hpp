#ifndef VL_UDP_SERVER_CONFIG_HPP
#define VL_UDP_SERVER_CONFIG_HPP

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include <stdint.h>

#include "base/rapidxml.hpp"

#include "command.hpp"

namespace vl
{
	
namespace udp
{

class ServerConfig
{
public:
	ServerConfig();
	
	virtual ~ServerConfig();

	void setPort( uint16_t );

	uint16_t getPort( void )
	{ return _port; }

	void addCommand( vl::udp::CommandRefPtr cmd );

	vl::udp::CommandRefPtr getCommand( size_t i )
	{
		return _commands.at(i);
	}

private :
	uint16_t _port;

	std::vector<vl::udp::CommandRefPtr> _commands;
};

typedef boost::shared_ptr<ServerConfig> ServerConfigRefPtr;

class ServerConfigSerializer
{
public :
	ServerConfigSerializer( ServerConfigRefPtr config );

	~ServerConfigSerializer( void );

	void readData( char *xml_data );

private :
	void processServer( rapidxml::xml_node<> *xml_root );

	void processPacket( rapidxml::xml_node<> *xml_node );

	void processVector( rapidxml::xml_node<> *xml_node );

	void processAngleAxis( rapidxml::xml_node<> *xml_node );

	void processQuaternion( rapidxml::xml_node<> *xml_node );
};

}	// namespace udp

}	// namespace vl

#endif // VL_UDP_SERVER_CONFIG_HPP
