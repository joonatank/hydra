/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
 *
 *	Demo executable
 *	Supports loading the settings
 *	Supports listening to a port
 */

// Necessary for vl::exceptions
#include "base/exceptions.hpp"

// Necessary for settings
#include "eq_cluster/eq_settings.hpp"
// Necessary for creating equalizer nodes
#include "eq_cluster/nodeFactory.hpp"
// Necessary for equalizer client creation
#include "eq_cluster/client.hpp"

int main( const int argc, char** argv )
{

	std::ofstream log_file;

	eq::base::RefPtr< eqOgre::Client > client;

	bool error = false;
	try
	{
		// This seems to work with two nodes
		vl::SettingsRefPtr settings = eqOgre::getSettings( argc, argv );
		if( !settings )
		{ return -1; }

		eqOgre::NodeFactory nodeFactory;
/*
		// TODO test if this can work with relative path
		// Oh right it doesn't for autolaunched clients.
		// NOTE the log file needs to be set before any calls to Equalizer methods
		log_file.open( settings->getEqLogFilePath().c_str() );
		eq::base::Log::setOutput( log_file );
		std::cout << "Equalizer log file = " << settings->getEqLogFilePath()
			<< std::endl;
*/

		// This seems to work with two nodes
		vl::Args &arg = settings->getEqArgs();

		std::cout << "Equalizer arguments = " << settings->getEqArgs() << std::endl;

		int eq_argc = arg.size();
		char **eq_argv = arg.getData();

		// 1. Equalizer initialization
		if( !eq::init( eq_argc, eq_argv, &nodeFactory ) )
		{
			EQERROR << "Equalizer init failed" << std::endl;
			error = true;
		}

		// 2. initialization of local client node
		client = new eqOgre::Client( settings );
		if( !client->initLocal( eq_argc, eq_argv ) )
		{
			EQERROR << "client->initLocal failed" << std::endl;
			error = true;
		}
		if( !error )
		{
			error = !client->run();
			if( error )
			{ std::cerr << "Client run returned an error." << std::endl; }
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

	// Exit
	client = 0;
	eq::exit();

	if( error )
	{ std::cerr << "Application exited with an error." << std::endl; }
	else
	{ std::cerr << "Application exited fine." << std::endl; }

	return error ? EXIT_FAILURE : EXIT_SUCCESS;
}
