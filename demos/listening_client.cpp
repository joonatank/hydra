/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
 *
 *	Demo executable
 *	Supports loading the settings
 *	Supports listening to a port
 */

// Necessary for vl::exceptions
#include "base/exceptions.hpp"
// Necessary for vl::msleep
#include "base/sleep.hpp"
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
		eqOgre::SettingsRefPtr settings = eqOgre::getSettings( argc, argv );
		if( !settings )
		{ return -1; }

		settings->setLogDir( "logs" );
		eqOgre::NodeFactory nodeFactory;

		// TODO test if this can work with relative path
		// Oh right it doesn't for autolaunched clients.
		// NOTE the log file needs to be set before any calls to Equalizer methods
		log_file.open( settings->getEqLogFilePath().c_str() );
		eq::base::Log::setOutput( log_file );
		std::cout << "Equalizer log file = " << settings->getEqLogFilePath()
			<< std::endl;

		vl::Args &arg = settings->getEqArgs();

		std::cout << "Args = " << settings->getEqArgs() << std::endl;
		// 1. Equalizer initialization
		if( !eq::init( arg.size(), arg.getData(), &nodeFactory ) )
		{
			EQERROR << "Equalizer init failed" << std::endl;
			error = true;
		}

		// 2. initialization of local client node
		client = new eqOgre::Client( settings );
		if( !client->initLocal( arg.size(), arg.getData() ) )
		{
			EQERROR << "client->initLocal failed" << std::endl;
			error = true;
		}

		if( !client->initialise() )
		{
			EQERROR << "client->init failed" << std::endl;
			error = true;
		}
		if( !error )
		{
			uint32_t frame = 0;
			while( client->mainloop(++frame) )
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

	// Exit
	client = 0;
	eq::exit();
	return error ? EXIT_FAILURE : EXIT_SUCCESS;
}
