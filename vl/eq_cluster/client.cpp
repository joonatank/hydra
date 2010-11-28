#include "client.hpp"

#include "eq_cluster/config.hpp"

// Necessary for vl::Settings
#include "settings.hpp"
// Necessary for vl::msleep
#include "base/sleep.hpp"

eqOgre::Client::Client( vl::SettingsRefPtr settings )
	: _config(0), _settings(settings)
{}

eqOgre::Client::~Client(void )
{}


bool eqOgre::Client::run(void )
{
	// 1. connect to server
	eq::ServerPtr server = new eq::Server;
	if( !connectServer( server ))
	{
		EQERROR << "Can't open server" << std::endl;
		return false;
	}

	// 2. choose config
	eq::ConfigParams configParams;
	eq::Config* config = server->chooseConfig( configParams );

	if( !config )
	{
		EQERROR << "No matching config on server" << std::endl;
		disconnectServer( server );
		return false;
	}

	// 3. init config
	_config = dynamic_cast<Config *>(config);
	if( _config )
	{ _config->setSettings( _settings ); }
	else
	{ EQERROR << "Running without eqOgre::Config" << std::endl; }

	if( !config->init(0) )
	{
		server->releaseConfig( config );
		disconnectServer( server );
		return false;
	}

    // 4. run main loop
	uint32_t frame = 0;
	while( config->isRunning() )
    {
		config->startFrame(++frame);
		config->finishFrame();

		// Sleep a bit
		// TODO this should sleep enough to get a 60 fps but not more
		// FIXME this is really slow on Linux
		vl::msleep(1);
    }

	std::cerr << "Exiting the config." << std::endl;
	// 5. exit config
	config->exit();

	std::cerr << "Releasing config." << std::endl;
	// 6. cleanup and exit
	server->releaseConfig( config );
	std::cerr << "Disconnecting from Server" << std::endl;
	if( !disconnectServer( server ))
		EQERROR << "Client::disconnectServer failed" << std::endl;

	server = 0;

	return true;
}
