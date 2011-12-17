/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011-10 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-12
 *	@file application.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

/*
 *	Program starting point
 *	@todo should probably be replaced with just the main function
 *	or merge Config and Application classes and create the threads in the main
 */

#ifndef HYDRA_APPLICATION_HPP
#define HYDRA_APPLICATION_HPP

#include <boost/thread.hpp>

#include <stdint.h>

// Necessary for HYDRA_API
#include "defines.hpp"
// Necessary for Ref ptrs
#include "typedefs.hpp"
// Necessary for vl::ProgramOptions
#include "program_options.hpp"
#include "logger.hpp"
// Necessary for ExceptionMessage struct
#include "base/exceptions.hpp"

namespace vl
{

vl::config::EnvSettingsRefPtr getMasterSettings( vl::ProgramOptions const & options );
vl::Settings getProjectSettings( vl::ProgramOptions const &options );
vl::config::EnvSettingsRefPtr getSlaveSettings( vl::ProgramOptions const &options );


extern "C"
{
	// Application
	HYDRA_API vl::ExceptionMessage Hydra_Run(const int argc, char** argv);
}

class Config;

class HYDRA_API Application
{
public:
	/** @brief start the rendering threads and initialise the application
	 *	@param opt Options for this application
	 * 
	 *	Will create a rendering thread and start it running.
	 *	If this is the master node will also initialise the master node
	 *	and send messages to the rendering threads.
	 *
	 */
	Application(ProgramOptions const &opt);

	virtual ~Application( void );

	/// @brief progress a single frame
	/// @todo this is problematic because this both renders the scene and
	/// also handles communications, we might want to separate the two
	/// @return false if the application wants to quit
	bool progress(void);
	
	// blocks till the program has exited calling progress
	void run(void);

	/// @returns true if the application is still running
	bool isRunning(void) const;

	RendererInterface *getRenderer(void);

protected:
	void _mainloop(bool sleep);

	void _exit(void);

	void _init(vl::config::EnvSettingsRefPtr env, vl::Settings const &settings, 
		ProgramOptions const &opt);

	vl::ConfigRefPtr _master;

	vl::cluster::ClientRefPtr _slave_client;

	uint32_t _max_fps;

	std::vector<uint32_t> _spawned_processes;

	vl::Logger _logger;

};	// class Application

}	// namespace vl

#endif	// HYDRA_APPLICATION_HPP
