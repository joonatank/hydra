#ifndef HANDLER_HPP
#define HANDLER_HPP

#include <string>
#include <vector>

#include "udp_conf.hpp"

namespace vl
{
	
namespace udp
{

/**	Virtual base class for all handlers.
 *	Server calls handle with the packet data received.
 *	All actions are executed by the derived handler.
 */
class Handler
{
public:
	/// Pass packet id to the handler constructor, if id is -1 handler handles
	/// all packets. In other words all packets have the same signature.
	Handler( int packet_id );

	virtual ~Handler();

	void setPacket( udp::Packet const &packet );

	virtual void handle( std::vector<double> msg ) = 0;

protected :
	udp::Packet _packet;
	int _packet_id;
};	// class Handler

}	// namespace udp

}	// namespace vl

#endif // HANDLER_HPP
