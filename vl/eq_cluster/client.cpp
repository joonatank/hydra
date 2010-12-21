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

	// Used to measure the time for rendering
	eq::base::Clock clock;
	// Used to measure one frame time
	eq::base::Clock frame_clock;
	// Time used for rendering the frames
	// (well not excatly as there is lots of other stuff in there also)
	// but this does not have the sleeping time
	float rendering_time = 0;

    // 4. run main loop
	uint32_t frame = 0;
	clock.reset();
	while( config->isRunning() )
    {
		frame_clock.reset();

		config->startFrame(++frame);
		if( config->getError( ))
            EQWARN << "Error during frame start: " << config->getError()
                   << std::endl;
		config->finishFrame();

        while( hasCommands() ) // execute non-critical pending commands
        {
            processCommand();
        }
		
		config->handleEvents(); // process all pending events

		rendering_time += frame_clock.getTimed();
		// Sleep enough to get a 60 fps but no more
		// TODO the fps should be configurable
		vl::msleep( 16.66-frame_clock.getTimed() );

		// Print info every two hundred frame
		if( (frame % 200) == 0 )
		{
			// TODO the logging should probably go to stats file and optionally
			// to the console
			// TODO also there should be possibility to reset the clock
			// for massive parts in a scene for example
			std::cout << "Avarage fps = " << frame/(clock.getTimed()/1000)
				<< ". took " << rendering_time/frame
				<< " ms in avarage for rendering one frame."
				<< " Took " << clock.getTimed() << "ms to render "
				<< frame << " frames." << std::endl;
		}
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
