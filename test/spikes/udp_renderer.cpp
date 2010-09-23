// Standard headers
#include <iostream>

// UNIX headers
#include <time.h>

#include "udp/server.hpp"
#include "udp/print_command.hpp"
#include <base/sleep.hpp>

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

			vl::msleep(1);
		}
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception : " << e.what() << std::endl;
	}

	return 0;
}
