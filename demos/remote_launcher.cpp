/**
 *	Copyright (c) 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2012-03
 *	@file remote_launcher.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 */

#include "remote_launcher.hpp"

#include "base/filesystem.hpp"
#include "base/system_util.hpp"
#include "base/string_utils.hpp"

std::string getDefaultIniFile(void)
{
	fs::path p = vl::get_global_path(vl::GP_APP_DATA) / fs::path("\\Hydra\\hydra_remote_launcer.ini");
	return p.generic_string();
}

int install(void)
{
	std::cout << "Should install to " << vl::get_global_path(vl::GP_STARTUP) << std::endl;

	return 0;
}

Options getOptions(void)
{
	std::cout << "Getting config file stored in " << getDefaultIniFile() << std::endl;
	Options opt;
	opt.load_ini(getDefaultIniFile());
	
	return opt;
}

void
Options::save_ini(std::string const &path)
{
	/// @todo should we check if we can write there?
	if(!path.empty())
	{ _ini_file_path = path; }

	std::cout << "saving ini file to : " << _ini_file_path << std::endl;

	// Error checking and directory creation
	fs::path file(_ini_file_path);
	fs::path dir = file;
	dir.remove_filename();
	if(!fs::exists(dir))
	{
		std::cout << "Creating parent directory " << dir << std::endl;
		fs::create_directories(dir);
	}

	_save_ini();
}

void
Options::load_ini(std::string const &path)
{
	_ini_file_path = path;

	if(fs::exists(_ini_file_path))
	{
		std::cout << "Loading ini file in : " << _ini_file_path << std::endl;
		_parse_ini();
	}
}

void
Options::loadDefault(void)
{
	_ini_file_path = getDefaultIniFile();

	launcher_port = DEFAULT_LAUNCHER_PORT;
	hydra_port = DEFAULT_HYDRA_PORT;
}

void
Options::_parse_ini(void)
{
	// Create an empty property tree object
	using boost::property_tree::ptree;
	ptree pt;

	// Load the INI file into the property tree. If reading fails
	// (cannot open file, parse error), an exception is thrown.
	read_ini(_ini_file_path, pt);

	launcher_port = pt.get("launcher.port", DEFAULT_LAUNCHER_PORT);
	launcher_auto_start = pt.get("launcher.auto_start", false);
	hydra_port = pt.get("hydra.port", DEFAULT_HYDRA_PORT);
	hydra_exe = pt.get("hydra.exe", "");
	master_hostname = pt.get("hydra.master_hostname", "");
	slave_name = pt.get("hydra.slave_name", "");
}


void
Options::_save_ini(void)
{
	// Create an empty property tree object
	using boost::property_tree::ptree;
	ptree pt;

	pt.put("launcher.port", launcher_port);
	pt.put("launcher.auto_start", launcher_auto_start);
	pt.put("hydra.exe", hydra_exe);
	pt.put("hydra.master_hostname", master_hostname);
	pt.put("hydra.slave_name", slave_name);

	write_ini(_ini_file_path, pt);
}

bool
Options::valid(void) const
{
	if(slave_name.empty() || hydra_exe.empty())
	{ return false; }

	return true;
}

std::ostream &operator<<(std::ostream &os, Options const &opt)
{
	os << "Options : \n"
		<< " launcher.port = " << opt.launcher_port << "\n"
		<< " hydra.exe = " << opt.hydra_exe << "\n"
		<< " master hostname = " << opt.master_hostname << "\n"
		<< " slave name = " << opt.slave_name << std::endl;

	return os;
}

RemoteLauncher::RemoteLauncher(Options const &opt)
	: _socket(_io_service) //, boost::udp::endpoint(boost::udp::v4(), opt.launcher_port))
	, _options(opt)
	, _running(false)
{
	if(_options.launcher_auto_start)
	{
		setRunning(true);
	}

	kill_hydra_instances();
}

void
RemoteLauncher::start_hydra(std::string const server_address)
{
	std::cout << "Should start Hydra with master "
		<< server_address << std::endl;

	/// @todo use a more general parameter for the program name
	std::vector<std::string> params;
	// Add slave param
	params.push_back("--slave");
	params.push_back(_options.slave_name);
	// Add server param
	params.push_back("--server");
		
	params.push_back(server_address);
	// @todo use TMP dir for logging
	//params.push_back("--log_dir");
	//params.push_back(env->getLogDir());
	// Create the process
	uint32_t pid = vl::create_process(_options.hydra_exe, params, true);
}

void
RemoteLauncher::kill_hydra_instances(void)
{
	std::cout << "Should kill all Hydra instances." << std::endl;
}

void
RemoteLauncher::mainloop(void)
{
	_handle_messages();
}

bool
RemoteLauncher::isRunning(void) const
{
	return _running;
}

void
RemoteLauncher::setRunning(bool running)
{
	if(_running == running)
	{ return; }

	if(running)
	{
		_socket.open(boost::udp::v4());
		_init_socket();
		_socket.bind(boost::udp::endpoint(boost::udp::v4(), _options.launcher_port));
	}
	else
	{
		_socket.shutdown(boost::udp::socket::shutdown_both);
		_socket.close();
	}

	_running = running;
}

void
RemoteLauncher::install(void)
{
}

void
RemoteLauncher::_handle_messages(void)
{
	if(!isRunning())
	{ return; }

	while( _socket.available() )
	{
		std::vector<char> recv_buf( _socket.available() );

		boost::system::error_code error;

		_socket.receive_from( boost::asio::buffer(recv_buf),
			_master_endpoint, 0, error );

		if (error && error != boost::asio::error::message_size)
		{ throw boost::system::system_error(error); }

		std::string msg(&recv_buf[0], recv_buf.size());
	
		_handle_command(msg, _master_endpoint);
	}
}

void
RemoteLauncher::_handle_command(std::string const &cmd, boost::udp::endpoint const &sender)
{
	/// @todo add support for parameters in commands
	std::vector<std::string> parts;
	vl::break_string_down(parts, cmd, ' ');
	if(parts.at(0) == "start")
	{
		std::stringstream address;
		assert(parts.size() > 1);
		address << sender.address().to_string() << ":" << parts.at(1);
		start_hydra(address.str());
	}
	else if(parts.at(0) == "kill")
	{
		kill_hydra_instances();
	}
	else
	{
		std::cout << "Unknown command \"" << cmd << "\" received." << std::endl;
	}
}

void
RemoteLauncher::_init_socket(void)
{
	_socket.set_option(boost::asio::socket_base::broadcast(true));

	// force that we are only receiving messages from master.
	// @todo should be resolved before we are here	
	if(!_options.master_hostname.empty())
	{
		boost::system::error_code error;
		boost::udp::resolver resolver(_io_service);
		boost::udp::resolver::query query(_options.master_hostname, "");
		boost::udp::resolver_iterator iter = resolver.resolve(query, error);
					
		if(error)
		{
			std::cout << "Couldn't resolve the master hostname. Defaulting to NONE." << std::endl;
			_options.master_hostname.clear();
		}
		else
		{
			_master_endpoint = *iter;
		}
	}
}
