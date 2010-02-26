#include "../../udp_server.hpp"

unsigned int const PORT = 5333;
int main( int argc, char **argv )
{
	std::cout << "Starting server on port " << PORT << std::endl;
	vl::udp::Server server( PORT );

	while( 1 )
	{
		server.receive();
		std::auto_ptr<vl::server::Command> cmd = server.popCommand();
		while( cmd.get() )
		{
			std::cout << "Received command" << std::endl << *cmd;
			cmd = server.popCommand();
		}
	}
}
