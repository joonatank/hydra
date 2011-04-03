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

#include "renderer.hpp"

#include "logger.hpp"

#include "cluster/client.hpp"

/// -------------------------------- Global ----------------------------------
vl::EnvSettingsRefPtr vl::getMasterSettings( vl::ProgramOptions const &options )
{
	if( options.slave() )
	{ return vl::EnvSettingsRefPtr(); }

	fs::path env_path = options.environment_file;
	if( !fs::is_regular(env_path) )
	{
		// Valid names for the environment config:
		// hydra_env.xml, hydra.env
		// paths ${Program_dir}, ${Program_dir}/data
		std::vector<fs::path> paths;
		paths.push_back( options.program_directory );
		paths.push_back( options.program_directory + "/data" );
		for( size_t i = 0; i < paths.size(); ++i )
		{
			if( fs::exists(paths.at(i) / "hydra.env") )
			{
				env_path = paths.at(i) / "hydra.env";
				break;
			}
			else if( fs::exists(paths.at(i) / "hydra_env.xml") )
			{
				env_path = paths.at(i) / "hydra_env.xml";
				break;
			}
		}

	}

	if( !fs::is_regular(env_path) )
	{
		return vl::EnvSettingsRefPtr();
	}
	else
	{
		/// This point both env_path and project_path are valid
		vl::EnvSettingsRefPtr env( new vl::EnvSettings );

		/// Read the Environment config
		if( fs::is_regular(env_path) )
		{
			std::string env_data;
			env_data = vl::readFileToString( env_path.file_string() );
			// TODO check that the files are correct and we have good settings
			vl::EnvSettingsSerializer env_ser( env );
			env_ser.readString(env_data);
			env->setFile( env_path.file_string() );
		}

		env->setLogDir( options.log_dir );
		env->setLogLevel( (vl::LogLevel)(options.log_level) );

		return env;
	}
}

vl::Settings vl::getProjectSettings( vl::ProgramOptions const &options )
{
	if( options.slave() )
	{ return vl::Settings(); }

	vl::Settings settings;

	/// Read the Project Config
	if( fs::is_regular( options.project_file ) )
	{
		vl::ProjSettings proj;

		std::string proj_data;
		proj_data = vl::readFileToString( options.project_file );
		vl::ProjSettingsRefPtr proj_ptr( &proj, vl::null_deleter() );
		vl::ProjSettingsSerializer proj_ser( proj_ptr );
		proj_ser.readString(proj_data);
		proj.setFile( options.project_file );

		settings.setProjectSettings(proj);
	}

	std::string global_file = options.global_file;
	/// If the user did not set the global, we try to find it
	if( !fs::is_regular(global_file) )
	{
		// Try to find the global file from default paths
		// ${program_dir}/hydra.prj, ${program_dir}/hydra.xml,
		// ${program_dir}/data/hydra.xml, ${program_dir}/data/hydra.prj
		// ${program_dir}/data/global/hydra.xml, ${program_dir}/data/global/hydra.prj
		std::vector<fs::path> paths;
		paths.push_back( options.program_directory );
		paths.push_back( options.program_directory + "/data" );
		paths.push_back( options.program_directory + "/data/global" );
		for( size_t i = 0; i < paths.size(); ++i )
		{
			if( fs::exists(paths.at(i) / "hydra.prj") )
			{
				global_file = fs::path(paths.at(i) / "hydra.prj").file_string();
				break;
			}
			else if( fs::exists(paths.at(i) / "hydra.xml") )
			{
				global_file = fs::path(paths.at(i) / "hydra.xml").file_string();
				break;
			}
		}
	}


	/// Read the global config
	if( fs::is_regular(global_file) )
	{
		vl::ProjSettings global;

		std::string global_data;
		global_data = vl::readFileToString( global_file );
		vl::ProjSettingsRefPtr glob_ptr( &global, vl::null_deleter() );
		vl::ProjSettingsSerializer glob_ser( glob_ptr );
		glob_ser.readString(global_data);
		global.setFile( global_file );

		settings.addAuxilarySettings(global);
	}

	return settings;
}

vl::EnvSettingsRefPtr vl::getSlaveSettings( vl::ProgramOptions const &options )
{
	if( options.master() )
	{
		return vl::EnvSettingsRefPtr();
	}
	if( options.slave_name.empty() || options.server_address.empty() )
	{
		return vl::EnvSettingsRefPtr();
	}

	size_t pos = options.server_address.find_last_of(":");
	if( pos == std::string::npos )
	{ return vl::EnvSettingsRefPtr(); }
	std::string hostname = options.server_address.substr(0, pos);
	uint16_t port = vl::from_string<uint16_t>( options.server_address.substr(pos+1) );

	vl::EnvSettingsRefPtr env( new vl::EnvSettings );
	env->setSlave();
	vl::EnvSettings::Server server(port, hostname);
	env->setServer(server);
	env->getMaster().name = options.slave_name;

	return env;
}


/// -------------------------------- Application -----------------------------
vl::Application::Application( vl::EnvSettingsRefPtr env, vl::Settings const &settings, vl::Logger &logger )
	: _master()
	, _slave_client()
{
	assert( env );

	std::cout << "vl::Application::Application : name = " << env->getName() << std::endl;

	// We should hand over the Renderer to either client or config
	vl::RendererInterfacePtr renderer( new Renderer(env->getName()) );

	if( env->isMaster() )
	{
		_master.reset(new vl::Config( settings, env, logger, renderer ));
		_master->init();
	}
	else
	{
		char const *host = env->getServer().hostname.c_str();
		uint16_t port = env->getServer().port;
		_slave_client.reset(new vl::cluster::Client(host, port, renderer));
	}
}

vl::Application::~Application( void )
{}


void
vl::Application::run( void )
{
	if(_master)
	{
		// run main loop
		uint32_t frame = 0;
		while( _master->isRunning() )
		{
			_render( ++frame );
		}

		_master->exit();
	}
	else
	{
		assert(_slave_client);
		while( _slave_client->isRunning() )
		{
			_slave_client->mainloop();

			/// Windows can sleep too long if allowed to
#ifdef WIN32
			if( !_slave_client->isRendering() )
			{ vl::msleep(1); }
#else
			/// Linux needs a sleeping in the mainloop
			vl::msleep(0);
#endif
		}

		// TODO add clean exit
	}
}

/// ------------------------------- Private ------------------------------------
void
vl::Application::_render( uint32_t const frame )
{
	Ogre::Timer timer;

	// target FPS
	// @todo should be configurable from EnvSettings
	const double FPS = 60;

	_master->render();

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
