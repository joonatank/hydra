/**
 *	Copyright (c) 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2012-03
 *	@file remote_launcher.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

#ifndef REMOTE_LAUNCHER_HPP
#define REMOTE_LAUNCHER_HPP

// Used for ini file parsing
// because program_options uses Linux config file syntax
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/foreach.hpp>

#include <boost/asio.hpp>

#include <cstdint>

const uint16_t DEFAULT_LAUNCHER_PORT = 9556;
const uint16_t DEFAULT_HYDRA_PORT = 4699;


namespace boost
{
	using boost::asio::ip::udp;
}


/// Class Options
struct Options
{
	Options(void)
	{
		loadDefault();
	}

	// @param path if empty uses the last load path
	void save_ini(std::string const &path = "");

	void load_ini(std::string const &path);

	/// @brief load the default configuration, useful when no ini file
	void loadDefault(void);

	void _parse_ini(void);

	void _save_ini(void);

	bool valid(void) const;

	uint16_t launcher_port;
	bool launcher_auto_start;

	uint16_t hydra_port;
	std::string master_hostname;
	std::string slave_name;
	std::string hydra_exe;

	std::string _ini_file_path;
};

/// Global functions
std::string getDefaultIniFile(void);

Options getOptions(void);

std::ostream &operator<<(std::ostream &os, Options const &opt);

class RemoteLauncher
{
public :
	RemoteLauncher(Options const &opt, std::string const &exe_path);

	void start_hydra(std::string const server_address);

	/// @brief kill all hydra instances in this computer
	/// This is to ensure that we are managing them
	void kill_hydra_instances(void);

	void mainloop(void);
	
	Options &getOptions(void)
	{ return _options; }

	Options const &getOptions(void) const
	{ return _options; }

	std::string const &getExePath(void) const
	{ return _exe_path; }

	bool isRunning(void) const;

	void setRunning(bool running);

private :
	void _handle_messages(void);

	void _handle_command(std::string const &cmd, boost::udp::endpoint const &sender);

	void _init_socket(void);

	bool _running;

	boost::asio::io_service _io_service;
	boost::udp::socket _socket;
	boost::udp::endpoint _master_endpoint;

	Options _options;

	std::string _exe_path;
};

#endif	// REMOTE_LAUNCHER_HPP
