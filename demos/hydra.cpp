/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2010-11
 *
 */

// Necessary for vl::exceptions
#include "base/exceptions.hpp"

// Necessary for settings
#include "base/envsettings.hpp"
#include "base/projsettings.hpp"

// Necessary for client creation
#include "application.hpp"

#include "base/string_utils.hpp"

#include "settings.hpp"

#include "logger.hpp"

#include <OGRE/OgreException.h>

#include "system_tools/message_box.hpp"

int main( const int argc, char** argv )
{
	std::string exception_msg;
	vl::ProgramOptions options;
	try
	{
		// Options need to be parsed before creating logger because logger
		// is designed to redirect logging out of console
		options.parseOptions(argc, argv);

		vl::Logger logger;
		logger.setOutputFile(options.getOutputFile());

		vl::EnvSettingsRefPtr env;
		vl::Settings settings;
		if( options.master() )
		{
			env = vl::getMasterSettings(options);
			settings = vl::getProjectSettings(options);
		}
		else
		{
			env = vl::getSlaveSettings(options);
		}

		// 2. initialization of local client node
		if( !env )
		{ return -1; }

		if( env->isMaster() && settings.empty() )
		{ return -1; }

		vl::Application client( env, settings, logger );

		client.run();
	}
	catch(vl::exception const &e)
	{
		exception_msg = "VL Exception : \n" + boost::diagnostic_information<>(e);
	}
	catch(boost::exception const &e)
	{
		exception_msg = "Boost Exception : \n"+ boost::diagnostic_information<>(e);
	}
	catch(Ogre::Exception const &e)
	{
		exception_msg = "Ogre Exception: \n" + std::string(e.what());
	}
	catch( std::exception const &e )
	{
		exception_msg = "STD Exception: \n" + std::string(e.what()); 
	}
	catch( ... )
	{
		exception_msg = std::string("An exception of unknow type occured.");
	}

	if(!exception_msg.empty())
	{
		std::string title;
		if(options.master())
		{ title = "Master"; }
		else
		{ title = "Slave"; }
		/// @todo We need a decent name function to options
		title += " " + options.slave_name + " ERROR";

		/// Show the exception message
		vl::MessageDialog dialog(title, exception_msg);
	}

	/// @fixme Debug version hangs on return
	return 0;
}
