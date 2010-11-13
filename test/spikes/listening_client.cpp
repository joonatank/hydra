/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
 *
 */

#include "base/exceptions.hpp"
#include "base/sleep.hpp"
#include "eq_cluster/eq_settings.hpp"
#include "eq_cluster/nodeFactory.hpp"

// Test helpers
#include "client_fixtures.hpp"

std::string const PROJECT_NAME( "listening_client" );

int main( const int argc, char** argv )
{
	bool error = false;
	try
	{
		ListeningClientFixture fix;

		eqOgre::SettingsRefPtr settings = eqOgre::getSettings( argc, argv );
		if( !settings )
		{ return -1; }

		settings->setLogDir( "logs" );
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
	catch( ... )
	{
		error = true;
	}

	return error ? EXIT_FAILURE : EXIT_SUCCESS;
}
