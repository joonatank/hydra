#include "udp_conf.hpp"

vl::udp::UdpConf::UdpConf()
{

}

vl::udp::UdpConf::~UdpConf()
{

}

bool
vl::udp::UdpConf::singlePacket( void )
{
	return _packets.size() == 1 && _packets.at(0)._id == -1;
}

vl::udp::Packet const &
vl::udp::UdpConf::getSinglePacket( void )
{
	if( !singlePacket() )
	{
		return udp::Packet();
	}
	else
	{
		return _packets.at(0);
	}
}

vl::udp::UdpConfSerializer::UdpConfSerializer ( boost::shared_ptr< udp::UdpConf > conf )
{

}

vl::udp::UdpConfSerializer::~UdpConfSerializer ( void )
{

}

void
vl::udp::UdpConfSerializer::readData ( char* xml_data )
{

}

void
vl::udp::UdpConfSerializer::processAngleAxis ( rapidxml::xml_node< char >* xml_node )
{

}

void
vl::udp::UdpConfSerializer::processPacket ( rapidxml::xml_node< char >* xml_node )
{

}

void
vl::udp::UdpConfSerializer::processQuaternion ( rapidxml::xml_node< char >* xml_node )
{

}

void
vl::udp::UdpConfSerializer::processServer ( rapidxml::xml_node< char >* xml_root )
{

}

void
vl::udp::UdpConfSerializer::processVector ( rapidxml::xml_node< char >* xml_node )
{

}
