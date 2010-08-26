#include "handler.hpp"

#include <iostream>

vl::udp::Handler::Handler( int packet_id )
	: _packet_id( packet_id )
{
}

vl::udp::Handler::~Handler()
{
}

void
vl::udp::Handler::setPacket(udp::Packet const &packet)
{
	_packet = packet;
}