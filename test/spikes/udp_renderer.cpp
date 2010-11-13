/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
 *
 */

// Standard headers
#include <iostream>

// Library includes
#include "udp/server.hpp"
#include "udp/print_command.hpp"
#include "base/sleep.hpp"
#include "base/exceptions.hpp"
#include "eq_cluster/eq_settings.hpp"
#include "eq_cluster/nodeFactory.hpp"

// Test includes
#include "client_fixtures.hpp"

std::string const PROJECT_NAME( "udp_renderer" );
uint16_t const PORT = 2244;

/** The message is of form
  * Angle joint1
  * Angle joint2
  * Angle joint3
  * Position amr1
  * Position arm2
  */
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

		// NOTE this code is completely broken, DONT TOUCH
		for (;;)
		{
			server.mainloop();

			vl::msleep(1);
		}

		ListeningClientFixture fix;

		eqOgre::SettingsRefPtr settings = eqOgre::getSettings( argc, argv );
		eqOgre::NodeFactory nodeFactory;
		error = !fix.init( settings, &nodeFactory );

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
		error = true;
	}
	catch( std::exception const &e )
	{
		std::cerr << "STD Exception: " << e.what() << std::endl;
		error = true;
	}

	return error ? EXIT_FAILURE : EXIT_SUCCESS;
}
