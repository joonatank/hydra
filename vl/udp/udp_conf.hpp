#ifndef UDP_CONF_HPP
#define UDP_CONF_HPP

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

class Packet
{
public :
	Packet( int16_t id = -1 )
		: _id(id)
	{}

	void addCommand( boost::shared_ptr<udp::Command> cmd )
	{ _commands.push_back( cmd ); }

	int16_t _id;
	std::vector< boost::shared_ptr<udp::Command> > _commands;
};

class UdpConf
{
public:
	UdpConf();
	
	virtual ~UdpConf();

	void setPort( uint16_t );

	uint16_t getPort( void );

	void addPacket( udp::Packet );

	udp::Packet const &getPacket( size_t i )
	{
		return _packets.at(i);
	}

	/// Returns true if only one type of packets are allowed
	bool singlePacket( void );

	udp::Packet const &getSinglePacket( void );

private :
	uint16_t _port;

	std::vector<Packet> _packets;
};

class UdpConfSerializer
{
public :
	UdpConfSerializer( boost::shared_ptr<UdpConf> conf );

	~UdpConfSerializer( void );

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

#endif // UDP_CONF_HPP
