#ifndef VL_UDP_PRINT_HANDLER_HPP
#define VL_UDP_PRINT_HANDLER_HPP

#include "handler.hpp"

namespace vl
{

namespace udp
{

class PrintHandler : public vl::udp::Handler
{
public :
	PrintHandler( int packet_id = -1 );

	virtual ~PrintHandler();

	virtual void handle( std::vector<double> msg );
};

}	// namespace udp

}	// namespace vl

#endif