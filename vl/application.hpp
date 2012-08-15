/**
 *	Copyright (c) 2010 - 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-12
 *	@file application.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

/*
 *	Program starting point
 *	@todo should probably be replaced with just the main function
 *	or merge Config and Application classes and create the threads in the main
 */

#ifndef HYDRA_APPLICATION_HPP
#define HYDRA_APPLICATION_HPP

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
vl::config::EnvSettingsRefPtr getSlaveSettings( vl::ProgramOptions const &options );


extern "C"
{
	// Application
	HYDRA_API void Hydra_Run(const int argc, char** argv, vl::ExceptionMessage *msg);
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
	Application(void);

	virtual ~Application( void );

	void init(ProgramOptions const &opt);

	/// @brief progress a single frame
	/// @todo this is problematic because this both renders the scene and
	/// also handles communications, we might want to separate the two
	/// @return false if the application wants to quit
	bool progress(void);
	
	// blocks till the program has exited calling progress
	void run(void);

	virtual void exit(void) = 0;

	/// @returns true if the application is still running
	virtual bool isRunning(void) const = 0;

	virtual RendererInterface *getRenderer(void) const = 0;

	virtual GameManagerPtr getGameManager(void) const = 0;

	static ApplicationUniquePtr create(ProgramOptions const &opt);
	
	// Private virtual overrides
private :
	virtual void _mainloop(bool sleep) = 0;

	virtual void _do_init(vl::config::EnvSettingsRefPtr env, ProgramOptions const &opt) = 0;

// Data
protected :

	vl::Logger *_logger;

};	// class Application

}	// namespace vl

#endif	// HYDRA_APPLICATION_HPP
