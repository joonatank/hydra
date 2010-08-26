// Standard headers
#include <iostream>

// UNIX headers
#include <time.h>

#include "udp/server.hpp"
#include "udp/print_handler.hpp"
#include "udp/print_command.hpp"

uint16_t const PORT = 2244;

int main(int argc, char **argv)
{
    std::cout << "Starting UDP server on port " << PORT << std::endl;

	try
	{
		vl::udp::Server server( PORT );

		// Create handler with two Vectors and one Quaternion
		boost::shared_ptr<vl::udp::Handler> handler( new vl::udp::PrintHandler );
		vl::udp::Packet packet;
		boost::shared_ptr<vl::udp::Command> cmd( new vl::udp::PrintCommand("setPosition", "feet" ) );
		packet.addCommand( cmd );
		cmd.reset( new vl::udp::PrintCommand("setPosition", "feet" ) );
		cmd.reset( new vl::udp::PrintCommand("setQuaternion", "feet" ) );
		handler->setPacket( packet );
		server.setHandler( handler );

		for (;;)
		{
			server.mainloop();

			// Sleep
			timespec tv;
			tv.tv_sec = 0;
			tv.tv_nsec = 1e3;
			::nanosleep( &tv, 0 );
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}
