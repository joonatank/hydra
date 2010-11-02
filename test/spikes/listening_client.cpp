
#include "dotscene_loader.hpp"
#include "base/exceptions.hpp"
#include "base/sleep.hpp"
#include "fake_tracker.hpp"
#include "vrpn_tracker.hpp"

// Crashes channel.h for sure
// Seems like including vmmlib/vector.h or quaternion.h will crash channel (vmmlib/frustum.h)
#include "math/conversion.hpp"

// Test helpers
#include "../test_helpers.hpp"
#include "client_fixtures.hpp"

std::string const PROJECT_NAME( "listening_client" );

int main( const int argc, char** argv )
{
	bool error = false;
	try
	{
		ListeningClientFixture fix;

		eqOgre::SettingsRefPtr settings = getSettings( argv[0], PROJECT_NAME );
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
	catch( ... )
	{
		error = true;
	}

	return error ? EXIT_FAILURE : EXIT_SUCCESS;
}
