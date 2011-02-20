/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2010-12
 *
 */
#include "application.hpp"

#include "eq_cluster/config.hpp"

// Necessary for vl::Settings
#include "settings.hpp"
// Necessary for vl::msleep
#include "base/sleep.hpp"

#include "game_manager.hpp"

#include "distrib_resource_manager.hpp"
#include "pipe.hpp"

/// -------------------------------- Global ----------------------------------
vl::EnvSettingsRefPtr vl::getMasterSettings( vl::ProgramOptions const &options )
{
	if( options.slave() )
	{ return vl::EnvSettingsRefPtr(); }

	if( options.environment_file.empty() )
	{ return vl::EnvSettingsRefPtr(); }

	/// This point both env_path and project_path are valid
	vl::EnvSettingsRefPtr env( new vl::EnvSettings );

	/// Read the Environment config
	if( fs::exists( options.environment_file ) )
	{
		std::string env_data;
		env_data = vl::readFileToString( options.environment_file );
		// TODO check that the files are correct and we have good settings
		vl::EnvSettingsSerializer env_ser( env );
		env_ser.readString(env_data);
		env->setFile( options.environment_file );
	}

	env->setLogDir( options.log_dir );
	env->setExePath( options.exe_path );
	env->setVerbose( options.verbose );

	return env;
}

vl::Settings vl::getProjectSettings( vl::ProgramOptions const &options )
{
	if( options.slave() )
	{
		std::cerr << "Trying to get projects for a slave configuration."
			<< std::endl;
		return vl::Settings();
	}

	vl::Settings settings;

	/// Read the Project Config
	if( fs::is_regular( options.project_file ) )
	{
		std::cout << "Reading project file." << std::endl;
		vl::ProjSettings proj;

		std::string proj_data;
		proj_data = vl::readFileToString( options.project_file );
		vl::ProjSettingsRefPtr proj_ptr( &proj, vl::null_deleter() );
		vl::ProjSettingsSerializer proj_ser( proj_ptr );
		proj_ser.readString(proj_data);
		proj.setFile( options.project_file );

		settings.setProjectSettings(proj);
	}

	/// Read the global config
	if( fs::is_regular( options.global_file ) )
	{
		vl::ProjSettings global;
		std::cout << "Reading global file." << std::endl;

		std::string global_data;
		global_data = vl::readFileToString( options.global_file );
		vl::ProjSettingsRefPtr glob_ptr( &global, vl::null_deleter() );
		vl::ProjSettingsSerializer glob_ser( glob_ptr );
		glob_ser.readString(global_data);
		global.setFile( options.global_file );

		settings.addAuxilarySettings(global);
	}

	return settings;
}

vl::EnvSettingsRefPtr vl::getSlaveSettings( vl::ProgramOptions const &options )
{
	if( options.master() )
	{
		std::cout << "Slave configuration with master options?" << std::endl;
		return vl::EnvSettingsRefPtr();
	}
	if( options.slave_name.empty() || options.server_address.empty() )
	{
		std::cout << "Slave configuration without all the parameters." << std::endl;
		return vl::EnvSettingsRefPtr();
	}

	size_t pos = options.server_address.find_last_of(":");
	if( pos == std::string::npos )
	{ return vl::EnvSettingsRefPtr(); }
	std::string hostname = options.server_address.substr(0, pos);
	std::cout << "Using server hostname = " << hostname;
	uint16_t port = vl::from_string<uint16_t>( options.server_address.substr(pos+1) );
	std::cout << " and port = " << port << "." << std::endl;

	vl::EnvSettingsRefPtr env( new vl::EnvSettings );
	env->setSlave();
	vl::EnvSettings::Server server(port, hostname);
	env->setServer(server);
	env->getMaster().name = options.slave_name;

	return env;
}


/// -------------------------------- Application -----------------------------
vl::Application::Application( vl::EnvSettingsRefPtr env, 
								  vl::Settings const &settings )
	: _env(env)
	, _game_manager(0)
	, _config(0)
	, _pipe_thread(0)
{
	assert( env );
	std::cout << "vl::Application::init" << std::endl;
	if( env->isMaster() )
	{
		// TODO GameManager and ResourceManager should be created in Config
		_game_manager = new vl::GameManager;
		_createResourceManager( settings, env );

		// TODO this should be done in python
		std::string song_name("The_Dummy_Song.ogg");
		_game_manager->createBackgroundSound(song_name);

		_config = new vl::Config( _game_manager, settings, env );
	}
}

vl::Application::~Application( void )
{
	std::cout << "vl::Application::~Application" << std::endl;
	
	delete _pipe_thread;
	delete _game_manager;
	delete _config;

	std::cout << "vl::Application::~Application : DONE" << std::endl;
}


void
vl::Application::run( void )
{
	std::cout << "vl::Application::run" << std::endl;

	if( _env->isMaster() )
	{
		assert( _config );
		// 4. run main loop
		uint32_t frame = 0;
		while( _config->isRunning() )
		{
			_render( ++frame );
		}
	}
	_exit();

	std::cout << "vl::Application::run : DONE" << std::endl;
}

void
vl::Application::init( void )
{
	if( _env->isMaster() )
	{
		assert( _config );
		// NOTE This needs to be called before creating the Pipe thread
		// probably because the Server the Pipe thread connects to has to be
		// created before connecting to it.
		_config->init();
	}

	// Put the pipe thread spinning on both master and slave
	assert( !_pipe_thread );
	std::cout << "name = " << _env->getMaster().name << std::endl;
	vl::PipeThread pipe( _env->getMaster().name,
						  _env->getServer().hostname,
						  _env->getServer().port );
	_pipe_thread = new boost::thread( pipe );
}

/// ------------------------------- Private ------------------------------------
void
vl::Application::_exit(void )
{
	if( _config )
	{ _config->exit(); }

	// Wait till the Pipe thread has received the shutdown message and is finished
	_pipe_thread->join();
}

void
vl::Application::_render( uint32_t const frame )
{
	Ogre::Timer timer;

	// target FPS
	// @todo should be configurable from EnvSettings
	const double FPS = 60;

	_config->render();

	double time = double(timer.getMicroseconds())/1000;

	// Sleep enough to get a 60 fps but no more
	// NOTE Of course because the converting to uint makes the time really huge
	// Of course the next question is why the time is negative without rendering data
	// i.e. why it would take more than 16.66ms for rendering a frame without data
	// TODO the fps should be configurable
	double sleep_time = 1000.0/FPS - time;
	if( sleep_time > 0 )
	{ vl::msleep( (uint32_t)sleep_time ); }
}

void
vl::Application::_createResourceManager( vl::Settings const &settings, vl::EnvSettingsRefPtr env )
{
	std::cout << "Initialising Resource Manager" << std::endl;

	std::cout << "Adding project directories to resources. "
		<< "Only project directory and global directory is added." << std::endl;

	std::vector<std::string> paths = settings.getAuxDirectories();
	paths.push_back(settings.getProjectDir());
	for( size_t i = 0; i < paths.size(); ++i )
	{ _game_manager->getReourceManager()->addResourcePath( paths.at(i) ); }

	// TODO add case directory

	// Add environment directory, used for tracking configurations
	std::cout << "Adding ${environment}/tracking to the resources paths." << std::endl;
	std::string tracking_dir( env->getEnvironementDir() + "/tracking" );
	_game_manager->getReourceManager()->addResourcePath( tracking_dir );
}
