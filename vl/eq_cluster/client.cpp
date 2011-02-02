/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-12
 *
 */
#include "client.hpp"

#include "eq_cluster/config.hpp"

// Necessary for vl::Settings
#include "settings.hpp"
// Necessary for vl::msleep
#include "base/sleep.hpp"

#include "game_manager.hpp"

#include "distrib_resource_manager.hpp"
#include "pipe.hpp"

eqOgre::Client::Client( vl::EnvSettingsRefPtr env, vl::SettingsRefPtr settings )
	: _env(env),
	  _settings(settings),
	  _game_manager(new vl::GameManager ),
	  _config(0),
	  _pipe_thread(0),
	   _pipe(0)
{
	assert( env && settings );
}

eqOgre::Client::~Client(void )
{
	delete _pipe_thread;
	delete _pipe;
	delete _game_manager;
}


bool
eqOgre::Client::run( void )
{
	std::cout << "eqOgre::Client::run" << std::endl;
	// Put the pipe thread spinning on both master and slave
	assert( !_pipe && !_pipe_thread );
	_pipe = new eqOgre::Pipe( _env->getMaster().name,
							  _env->getServer().hostname,
							  _env->getServer().port );
	// TODO create a custom wrapper for Pipe
	_pipe_thread = new boost::thread( boost::ref(*_pipe) );

	if( _env->isMaster() )
	{
		assert( _config );
		_clock.reset();
		_rendering_time = 0;
		// 4. run main loop
		uint32_t frame = 0;
		while( _config->isRunning() )
		{
			_render( ++frame );
		}

		_exit();
	}

	return true;
}

void
eqOgre::Client::init( void )
{
	std::cout << "eqOgre::Client::init" << std::endl;
	if( _env->isMaster() )
	{
		_createResourceManager();
		_init();
		_config = new eqOgre::Config( _game_manager, _settings, _env );
		_config->init(0);
	}
}

/// ------------------------------- Private ------------------------------------
bool
eqOgre::Client::_init( void )
{
	// TODO this should be done in python and only on the master
	std::string song_name("The_Dummy_Song.ogg");
	_game_manager->createBackgroundSound(song_name);

	return true;
}

void
eqOgre::Client::_exit(void )
{
	if( _config )
	{ _config->exit(); }

	// Remove the pipe thread
	// TODO this should exit the thread cleanly with join, by sending a message
	// indicating an exit to the pipe thread from Config::exit
	_pipe_thread->interrupt();
}

void
eqOgre::Client::_render( uint32_t const frame )
{
	// target FPS
	const double FPS = 60;

	_frame_clock.reset();

	_config->startFrame(frame);
	_config->finishFrame();
// 	if( _config->getError() )
// 	{ EQWARN << "Error during frame start: " << _config->getError() << std::endl; }

	double time = double(_frame_clock.getMicroseconds())/1000;
	_rendering_time += time;
	// Sleep enough to get a 60 fps but no more
	// NOTE Of course because the converting to uint makes the time really huge
	// Of course the next question is why the time is negative without rendering data
	// i.e. why it would take more than 16.66ms for rendering a frame without data
	// TODO the fps should be configurable
	double sleep_time = 1000.0/FPS - time;
	if( sleep_time > 0 )
	{ vl::msleep( (uint32_t)sleep_time ); }

	// Print info every two hundred frame
	if( (frame % 200) == 0 )
	{
		// TODO the logging should probably go to stats file and optionally
		// to the console
		// TODO also there should be possibility to reset the clock
		// for massive parts in a scene for example
		std::cout << "Avarage fps = " << 200.0/(double(_clock.getMicroseconds())/1e6)
			<< ". took " << _rendering_time/200
			<< " ms in avarage for rendering one frame." << std::endl;

		// Reset the stats
		_rendering_time = 0;
		_clock.reset();
	}
}

void
eqOgre::Client::_createResourceManager(void )
{
	std::cout << "Initialising Resource Manager" << std::endl;

	std::cout << "Adding project directories to resources. "
		<< "Only project directory and global directory is added." << std::endl;

	_game_manager->getReourceManager()->addResourcePath( _settings->getProjectDir() );
	_game_manager->getReourceManager()->addResourcePath( _settings->getGlobalDir() );

	// TODO add case directory

	// Add environment directory, used for tracking configurations
	std::cout << "Adding ${environment}/tracking to the resources paths." << std::endl;
	std::string tracking_dir( _env->getEnvironementDir() + "/tracking" );
	_game_manager->getReourceManager()->addResourcePath( tracking_dir );
}
