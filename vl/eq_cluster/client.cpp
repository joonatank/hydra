#include "client.hpp"

#include "eq_cluster/config.hpp"

// Necessary for vl::Settings
#include "settings.hpp"
// Necessary for vl::msleep
#include "base/sleep.hpp"

eqOgre::Client::Client( vl::SettingsRefPtr settings )
	: _settings(settings), _resource_manager(new eqOgre::ResourceManager),
	  _audio_manager(0), _background_sound(0), _config(0)
{
	EQASSERT( settings );
}

eqOgre::Client::~Client(void )
{}


bool
eqOgre::Client::run(void )
{
	// 1. connect to server
	_server = new eq::Server;
	if( !connectServer( _server ))
	{
		EQERROR << "Can't open server" << std::endl;
		return false;
	}

	// 2. choose config
	eq::ConfigParams configParams;
	eq::Config* config = _server->chooseConfig( configParams );

	if( !config )
	{
		EQERROR << "No matching config on server" << std::endl;
		disconnectServer( _server );
		return false;
	}

	// 3. init config
	if( !_init( config ) )
	{ return false; }

	_clock.reset();
	_rendering_time = 0;
	// 4. run main loop
	uint32_t frame = 0;
	while( _config->isRunning() )
    {
		_render( ++frame );
    }

	_exit();

	return true;
}


void
eqOgre::Client::createBackgroundSound( std::string const &song_name )
{
	EQASSERT( _resource_manager );

	//Create an audio source and load a sound from a file
	std::string file_path;

	if( _resource_manager->findResource( song_name, file_path ) )
	{
		vl::Resource resource;
		_resource_manager->loadOggResource( song_name, resource );
		_background_sound = _audio_manager
			->createFromMemory("The_Dummy_Song", resource.get(), resource.size(), "ogg" );
	}
	else
	{
		EQERROR << "Couldn't find " << song_name << " from resources." << std::endl;
	}
}

void
eqOgre::Client::toggleBackgroundSound()
{
	if( !_background_sound )
	{
		EQERROR << "NO background sound to toggle." << std::endl;
		return;
	}

	if( _background_sound->isPlaying() )
	{ _background_sound->pause(); }
	else
	{ _background_sound->play2d(false); }
}


bool
eqOgre::Client::_init( eq::Config *config )
{
	_config = static_cast<Config *>(config);
	EQASSERT( dynamic_cast<Config *>(config) );

	/// Set parameters to the config
	_config->setSettings( _settings );
	_createResourceManager();
	_config->setResourceManager( _resource_manager );

	/// TODO Register data here

	if( !_config->init(0) )
	{
		_server->releaseConfig( config );
		disconnectServer( _server );
		return false;
	}

	_initAudio();

	std::string song_name("The_Dummy_Song.ogg");
	createBackgroundSound(song_name);

	return true;
}

void
eqOgre::Client::_exit(void )
{
	_exitAudio();

	EQINFO << "Exiting the config." << std::endl;
	// 5. exit config
	_config->exit();

	EQINFO << "Releasing config." << std::endl;
	// 6. cleanup and exit
	_server->releaseConfig( _config );
	EQINFO << "Disconnecting from Server" << std::endl;
	if( !disconnectServer( _server ))
	{ EQERROR << "Client::disconnectServer failed" << std::endl; }

	_server = 0;

}

void eqOgre::Client::_render( uint32_t frame )
{
	_frame_clock.reset();

	_config->startFrame(frame);
	_config->finishFrame();

	_rendering_time += _frame_clock.getTimed();
	// Sleep enough to get a 60 fps but no more
	// TODO the fps should be configurable
	vl::msleep( 16.66-_frame_clock.getTimed() );

	// Print info every two hundred frame
	if( (frame % 200) == 0 )
	{
		// TODO the logging should probably go to stats file and optionally
		// to the console
		// TODO also there should be possibility to reset the clock
		// for massive parts in a scene for example
		std::cout << "Avarage fps = " << frame/(_clock.getTimed()/1000)
			<< ". took " << _rendering_time/frame
			<< " ms in avarage for rendering one frame."
			<< " Took " << _clock.getTimed() << "ms to render "
			<< frame << " frames." << std::endl;
	}
}

void
eqOgre::Client::_createResourceManager(void )
{
	EQINFO << "Initialising Resource Manager" << std::endl;

	EQINFO << "Adding project directories to resources. "
		<< "Only project directory and global directory is added." << std::endl;

	_resource_manager->addResourcePath( _settings->getProjectDir() );
	_resource_manager->addResourcePath( _settings->getGlobalDir() );

	// TODO add case directory

	// Add environment directory, used for tracking configurations
	EQINFO << "Adding ${environment}/tracking to the resources paths." << std::endl;
	std::string tracking_dir( _settings->getEnvironementDir() + "/tracking" );
	_resource_manager->addResourcePath( tracking_dir );
}


void
eqOgre::Client::_initAudio( void )
{
	EQINFO << "Init audio." << std::endl;

	//Create an Audio Manager
	_audio_manager = cAudio::createAudioManager(true);
}

void
eqOgre::Client::_exitAudio(void )
{
	EQINFO << "Exit audio." << std::endl;

	//Shutdown cAudio
	if( _audio_manager )
	{
		_audio_manager->shutDown();
		cAudio::destroyAudioManager(_audio_manager);
	}
}
