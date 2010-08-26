// Standard headers
#include <iostream>

// UNIX headers
#include <time.h>

#include "udp/server.hpp"
#include "udp/print_command.hpp"

uint16_t const PORT = 2244;

int main(int argc, char **argv)
{
    std::cout << "Starting UDP server on port " << PORT << std::endl;

	try
	{
		vl::udp::Server server( PORT );

		boost::shared_ptr<vl::udp::Command> cmd( new vl::udp::PrintCommand("setPosition", "feet" ) );
		server.addCommand( cmd );
		cmd.reset( new vl::udp::PrintCommand("setPosition", "feet" ) );
		server.addCommand( cmd );
		cmd.reset( new vl::udp::PrintCommand("setQuaternion", "feet" ) );
		server.addCommand( cmd );

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
