#include "base/sleep.hpp"

#include "../test_helpers.hpp"
#include "../debug.hpp"
#include <eq/client/channel.h>

#include "eq_ogre/ogre_root.hpp"
#include "settings.hpp"
#include "dotscene_loader.hpp"
#include "base/exceptions.hpp"
#include "eq_cluster/config.hpp"
#include "eq_cluster/window.hpp"
#include "eq_cluster/channel.hpp"
#include "fake_tracker.hpp"
#include "vrpn_tracker.hpp"
#include "base/helpers.hpp"

// Crashes channel.h for sure
// Seems like including vmmlib/vector.h or quaternion.h will crash channel (vmmlib/frustum.h)
#include "math/conversion.hpp"

std::string const PROJECT_NAME( "dotscene_loader" );

// This class is/should be the same in dotscene without trackign and with tracking
class NodeFactory : public eq::NodeFactory
{
public:
	virtual eq::Window *createWindow( eq::Pipe *parent )
	{ return new eqOgre::Window( parent ); }

	virtual eq::Channel *createChannel( eq::Window *parent )
	{ return new eqOgre::Channel( parent ); }

	virtual eq::Config *createConfig( eq::ServerPtr parent )
	{ return new eqOgre::Config( parent ); }
};

struct DotSceneFixture
{
	DotSceneFixture( void )
		: config(0)
	{
	}

	~DotSceneFixture( void )
	{
		if( config )
		{ exit(); }
	}

	bool init( vl::SettingsRefPtr settings, eq::NodeFactory *nodeFactory )
	{
		InitFixture();
		if( !settings )
		{
			std::cerr << "No settings provided";
			return false;
		}

		// Create eq log file
		uint32_t pid = vl::getPid();
		std::stringstream ss;
		if( !settings->getLogDir().empty() )
		{ ss << settings->getLogDir() << "/"; }

		ss << PROJECT_NAME << "_eq_" << pid << ".log";
		log_file = std::ofstream( ss.str() );

		// Redirect logging
		eq::base::Log::setOutput( log_file );

		vl::Args &arg = settings->getEqArgs();
		
		// 1. Equalizer initialization
		if( !eq::init( arg.size(), arg.getData(), nodeFactory ))
		{
			EQERROR << "Equalizer init failed" << std::endl;
			return false;
		}

		// 2. get a configuration
		config = static_cast< eqOgre::Config * >( eq::getConfig( arg.size(), arg.getData() ) );
		if( config )
		{
			config->setSettings( settings );
			// 3. init config
			if( !config->init(0) )
			{
				EQERROR << "Config initialization failed: "
						<< config->getErrorMessage() << std::endl;
				return false;
			}
		}
		else
		{
			EQERROR << "Cannot get config" << std::endl;
			return false;
		}

		return true;
	}

	bool mainloop( const uint32_t frame )
	{
		// 4. run main loop
		if( config->isRunning() )
		{
			config->startFrame( frame );
			config->finishFrame();
			return true;
		}
		else
		{ return false; }
	}

	void exit( void )
	{
		// 5. exit config
		config->exit();

		// 6. release config
		eq::releaseConfig( config );

		config = 0;
		
		// 7. exit
		eq::exit();
	}

	std::ofstream log_file;

	eqOgre::Config *config;
};

// This function is the same in dotscene without trackign and with tracking
int main( const int argc, char** argv )
{
	bool error = false;
	try
	{
		DotSceneFixture fix;

		// TODO move to using InitData
		vl::SettingsRefPtr settings = getSettings(argv[0], "dotscene_loader");
		if( !settings )
		{
			std::cerr << "No test_conf.xml file found." << std::endl;
			return -1;
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
	}
	catch( std::exception const &e )
	{
		std::cerr << "STD Exception: " << e.what() << std::endl;
	}

	return error ? EXIT_FAILURE : EXIT_SUCCESS;
}
