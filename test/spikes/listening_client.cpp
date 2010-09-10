#include "base/sleep.hpp"

#include "../fixtures.hpp"
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

class Client : public eq::Client
{
public :
	Client( vl::SettingsRefPtr settings )
		: _settings( settings ), _config(0)
	{}

	virtual bool initLocal (const int argc, char **argv)
	{
		return eq::Client::initLocal( argc, argv );
	}

	bool init( void )
	{
//		std::cerr << "Client::init" << std::endl;

		// 1. connect to server
		_server = new eq::Server;
//		std::cerr << "Client::init - server created" << std::endl;
		if( !connectServer( _server ))
		{
			EQERROR << "Can't open server" << std::endl;
			return false;
		}
//		std::cerr << "Client::init - connected to server" << std::endl;

		// 2. choose config
		eq::ConfigParams configParams;
		_config = static_cast<eqOgre::Config*>(_server->chooseConfig( configParams ));

		if( !_config )
		{
			EQERROR << "No matching config on server" << std::endl;
			disconnectServer( _server );
			return false;
		}
//		std::cerr << "Client::init - config selected" << std::endl;

		// 3. init config
		_config->setSettings( _settings );

		if( !_config->init(0) )
		{
			EQERROR << "Config initialization failed: " 
					<< _config->getErrorMessage() << std::endl;
			_server->releaseConfig( _config );
			disconnectServer( _server );
			return false;
		}
//		std::cerr << "Client::init - config inited" << std::endl;

		return true;
	}

	bool mainloop( uint32_t frame )
	{
//		std::cerr << "Client::mainloop" << std::endl;

		// 4. run main loop
		if( _config->isRunning() )
		{
			_config->startFrame( frame );
			_config->finishFrame();
			return true;
		}
		else
		{ return false; }
	}

	bool exit( void )
	{
//		std::cerr << "Client::exit" << std::endl;

		_config->exit();
		_server->releaseConfig( _config );
		
		if( !disconnectServer( _server ))
			EQERROR << "Client::disconnectServer failed" << std::endl;

		_server = 0;

		return true;
	}

	virtual bool clientLoop( void )
	{
//		std::cerr << "Client::clientLoop" << std::endl;

//		if( isLocal() )
//			std::cerr << "In Client : Client is local " << std::endl;
//		else
//			std::cerr << "In Client : Client is not local" << std::endl;

		if( !isLocal() ) // execute only one config run
			return eq::Client::clientLoop();

		// else execute client loops 'forever'
		while( true ) // TODO: implement SIGHUP handler to exit?
		{
			if( !eq::Client::clientLoop( ))
				return false;
			EQINFO << "One configuration run successfully executed" << std::endl;
		}
		
		return true;
	}

	vl::SettingsRefPtr _settings;
	eqOgre::Config *_config;
	eq::ServerPtr _server;
};

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

struct ListeningClientFixture
{
	ListeningClientFixture( void )
	{
		uint32_t pid = vl::getPid();
		std::stringstream ss;
		ss << pid;
		std::string name = "render_test-" + ss.str() + ".log";
		log_file = std::ofstream( name );
	}

	~ListeningClientFixture( void )
	{
		exit();
	}

	bool init( vl::SettingsRefPtr settings, eq::NodeFactory *nodeFactory )
	{
//		std::cerr << "ListeningClientFixture::init" << std::endl;

		InitFixture();
		if( !settings )
		{
			std::cerr << "No settings provided";
			return false;
		}

		vl::Args &arg = settings->getEqArgs();
		
		eq::base::Log::setOutput( log_file );
		
		// 1. Equalizer initialization
		if( !eq::init( arg.size(), arg.getData(), nodeFactory ) )
		{
			EQERROR << "Equalizer init failed" << std::endl;
			return false;
		}

		// 2. initialization of local client node
		client = new ::Client( settings );
		if( !client->initLocal( arg.size(), arg.getData() ) )
		{
			EQERROR << "client->initLocal failed" << std::endl;
			return false;
		}

		if( !client->init() )
		{
			EQERROR << "client->init failed" << std::endl;
			return false;
		}

		return true;
	}

	bool mainloop( const uint32_t frame )
	{
//		std::cerr << "ListeningClientFixture::mainloop" << std::endl;
		return client->mainloop( frame );
	}

	void exit( void )
	{
		client->exit();
		client->exitLocal();

		client = 0;
		
		eq::exit();
	}

	std::ofstream log_file;

	eq::base::RefPtr< ::Client > client;
};

// This function is the same in dotscene without trackign and with tracking
int main( const int argc, char** argv )
{
	bool error = false;
	try
	{
		ListeningClientFixture fix;

		vl::SettingsRefPtr settings = getSettings(argv[0]);
		if( !settings )
		{
			std::cerr << "No test_conf.xml file found." << std::endl;
			return -1;
		}

		// Add the command line arguments
		for( int i = 1; i < argc; ++i )
		{
			settings->getEqArgs().add( argv[i] );
		}

		std::cerr << "Args = " << settings->getEqArgs() << std::endl;

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
