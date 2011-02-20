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
vl::Application::Application( vl::EnvSettingsRefPtr env, vl::Settings const &settings )
	: _config(0)
	, _pipe_thread(0)
{
	assert( env );

	if( env->isMaster() )
	{
		_config = new vl::Config( settings, env );
		// NOTE This needs to be called before creating the Pipe thread
		// probably because the Server the Pipe thread connects to has to be
		// created before connecting to it.
		_config->init();
	}

	// Put the pipe thread spinning on both master and slave
	std::cout << "vl::Application::Application : name = " << env->getMaster().name << std::endl;
	vl::PipeThread pipe( env->getMaster().name,
						 env->getServer().hostname,
						 env->getServer().port );
	_pipe_thread = new boost::thread( pipe );
}

vl::Application::~Application( void )
{
	std::cout << "vl::Application::~Application" << std::endl;
	
	delete _pipe_thread;
	delete _config;

	std::cout << "vl::Application::~Application : DONE" << std::endl;
}


void
vl::Application::run( void )
{
	std::cout << "vl::Application::run" << std::endl;

	if( _config )
	{
		// run main loop
		uint32_t frame = 0;
		while( _config->isRunning() )
		{
			_render( ++frame );
		}
		
		_config->exit();
	}

	// Wait till the Pipe thread has received the shutdown message and is finished
	_pipe_thread->join();

	std::cout << "vl::Application::run : DONE" << std::endl;
}

/// ------------------------------- Private ------------------------------------
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


