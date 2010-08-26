#include "server_config.hpp"

vl::udp::ServerConfig::ServerConfig()
{

}

vl::udp::ServerConfig::~ServerConfig()
{

}

void
vl::udp::ServerConfig::addCommand(vl::udp::CommandRefPtr cmd)
{
	_commands.push_back(cmd);
}

void
vl::udp::ServerConfig::setPort( uint16_t port )
{
	// TODO add checking that the port value is valid
	_port = port;
}


vl::udp::ServerConfigSerializer::ServerConfigSerializer ( vl::udp::ServerConfigRefPtr conf )
{

}

vl::udp::ServerConfigSerializer::~ServerConfigSerializer ( void )
{

}

void
vl::udp::ServerConfigSerializer::readData ( char* xml_data )
{

}

void
vl::udp::ServerConfigSerializer::processAngleAxis ( rapidxml::xml_node< char >* xml_node )
{

}

void
vl::udp::ServerConfigSerializer::processPacket ( rapidxml::xml_node< char >* xml_node )
{

}

void
vl::udp::ServerConfigSerializer::processQuaternion ( rapidxml::xml_node< char >* xml_node )
{

}

void
vl::udp::ServerConfigSerializer::processServer ( rapidxml::xml_node< char >* xml_root )
{

}

void
vl::udp::ServerConfigSerializer::processVector ( rapidxml::xml_node< char >* xml_node )
{

}
