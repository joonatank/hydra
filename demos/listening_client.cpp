/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2010-11
 *
 *	Demo executable
 *	Supports loading the settings
 *	Supports listening to a port
 */

// Necessary for vl::exceptions
#include "base/exceptions.hpp"

// Necessary for settings
#include "base/envsettings.hpp"
#include "base/projsettings.hpp"

// Necessary for client creation
#include "eq_cluster/application.hpp"

#include "base/string_utils.hpp"

#include "settings.hpp"

#include <OGRE/OgreException.h>

int main( const int argc, char** argv )
{
	try
	{
		vl::ProgramOptions options;
		options.parseOptions(argc, argv);
		vl::EnvSettingsRefPtr env;
		vl::Settings settings;
		if( options.master() )
		{
			std::cout << "Requested master configuration." << std::endl;
			env = vl::getMasterSettings(options);
			settings = vl::getProjectSettings(options);
		}
		else
		{
			std::cout << "Requested slave configuration." << std::endl;
			env = vl::getSlaveSettings(options);
		}

		// 2. initialization of local client node
		if( !env )
		{ return -1; }

		if( env->isMaster() && settings.empty() )
		{ return -1; }

		vl::Application client( env, settings );

		client.run();
	}
	catch( vl::exception &e )
	{
		std::cerr << "VL Exception : "<<   boost::diagnostic_information<>(e)
			<< std::endl;
		return -1;
	}
	catch( Ogre::Exception const &e)
	{
		std::cerr << "Ogre Exception: " << e.what() << std::endl;
		return -1;
	}
	catch( std::exception const &e )
	{
		std::cerr << "STD Exception: " << e.what() << std::endl;
		return -1;
	}
	catch( ... )
	{
		std::cerr << "An exception of unknow type occured." << std::endl;
		return -1;
	}

	return 0;
}
