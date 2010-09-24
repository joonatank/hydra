// Standard headers
#include <iostream>

// UNIX headers
#include <time.h>

// Library includes
#include "udp/server.hpp"
#include "udp/print_command.hpp"
#include "base/sleep.hpp"
#include "base/exceptions.hpp"

// Test includes
#include "client_fixtures.hpp"
#include "../test_helpers.hpp"

std::string const PROJECT_NAME( "udp_renderer" );
uint16_t const PORT = 2244;

int main(int argc, char **argv)
{
	bool error = false;
	try
	{
		std::cout << "Starting UDP server on port " << PORT << std::endl;
		vl::udp::Server server( PORT );

		// TODO new message should be of form 3 x angle with predefined axis
		// and 3 x position
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

		ListeningClientFixture fix;

		eqOgre::InitData init_data = ::getInitData( argv[0], PROJECT_NAME );

		vl::SettingsRefPtr settings = init_data.getSettings();
		if( !settings )
		{
			std::cerr << "No test_conf.xml file found." << std::endl;
			return -1;
		}

		// Add the command line arguments
		for( int i = 1; i < argc; ++i )
		{
			settings->getEqArgs().add(argv[i] );
		}

		::NodeFactory nodeFactory;

		error = !fix.init( init_data, &nodeFactory );

		if( !error )
		{
			uint32_t frame = 0;
			while( fix.mainloop(++frame) )
			{
				vl::msleep(1);
			}
		}
	}
	catch( vl::exception &e )
	{
		std::cerr << "VL Exception : "<<   boost::diagnostic_information<>(e)
			<< std::endl;
		error = true;
	}
	catch( Ogre::Exception const &e)
	{
		std::cerr << "Ogre Exception: " << e.what() << std::endl;
	}
	catch( std::exception const &e )
	{
		std::cerr << "STD Exception: " << e.what() << std::endl;
	}

	return error ? EXIT_FAILURE : EXIT_SUCCESS;
}
