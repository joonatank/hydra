/**
 *	Copyright (c) 2010 - 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-12
 *	@file application.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

// Interface
#include "application.hpp"

// Necessary for checking architecture
#include "defines.hpp"
// Necessary for creating the Logger
#include "logger.hpp"

#include "base/string_utils.hpp"

#ifdef HYDRA_LINUX
// Necessary for fork
#include <unistd.h>
#endif

// Necessaru for Hydra_RUN
#include "master.hpp"
#include "slave.hpp"
// Necessary for hiding system console
#include "base/system_util.hpp"

// Compile time created header
#include "revision_defines.hpp"

// Necessary for printing Ogre exceptions
#include <OGRE/OgreException.h>

void
vl::Hydra_Run(const int argc, char** argv, vl::ExceptionMessage *msg)
{
	std::clog << "Starting Hydra : " << HYDRA_REVISION << std::endl;
	std::string exception_msg;
	vl::ProgramOptions options;
	try
	{
		// Does not set exception message 
		// because returns false for valid configuration
		// when user requested information and not really running the app.
		// If the options are incorrect this will throw instead.
		if(!options.parseOptions(argc, argv))
		{ return; }

		// File doesn't exist (checked earlier)
		if(!options.getLogDir().empty() && !fs::exists(options.getLogDir()))
		{
			fs::create_directory(options.getLogDir());
		}

		// Otherwise the file exists and it's a directory
		std::cout << "Using log file: " << options.getLogFile() << std::endl;

		vl::ApplicationUniquePtr app = Application::create(options);
		// @todo this doesn't allow an exception to be thrown through it
		// we need to use some other method
		// this is true for exception thrown by our GLRenderer

		app->run();
	}
	catch(vl::exception const &e)
	{
		exception_msg = "VL Exception : \n" + boost::diagnostic_information<>(e);
	}
	catch(boost::exception const &e)
	{
		exception_msg = "Boost Exception : \n"+ boost::diagnostic_information<>(e);
	}
	catch(Ogre::Exception const &e)
	{
		exception_msg = "Ogre Exception: \n" + std::string(e.what());
	}
	catch( std::exception const &e )
	{
		exception_msg = "STD Exception: \n" + std::string(e.what()); 
	}
	catch( ... )
	{
		exception_msg = std::string("An exception of unknow type occured.");
	}

	// Only set exception message if we have one
	if(msg)
	{
		if(!exception_msg.empty())
		{
			if(options.master())
			{ msg->title = "Master"; }
			else
			{ msg->title = "Slave"; }
			/// @todo We need a decent name function to options
			msg->title += " " + options.slave_name + " ERROR";

			msg->message = exception_msg;
		}
		// Clear
		else
		{ msg->clear();}
	}
}

/// -------------------------------- Application -----------------------------
vl::Application::Application(void)
	: _logger(0)
{
}

vl::Application::~Application( void )
{
	delete _logger;
}

vl::ApplicationUniquePtr
vl::Application::create(ProgramOptions const &opt)
{
	ApplicationUniquePtr ptr;
	if(opt.master())
	{ ptr.reset(new vl::Master()); }
	else
	{ ptr.reset(new vl::Slave()); }
	
	ptr->init(opt);

	return ptr;
}

bool
vl::Application::progress(void)
{
	// Necessary to catch exceptions here because our Editor can't handle them
	try {
		if(isRunning())
		{
			_mainloop(false);
		}
	}
	catch(vl::exception const &e)
	{
		std::cout << "VL Exception : \n" << boost::diagnostic_information<>(e) << std::endl;
		return false;
	}
	catch(boost::exception const &e)
	{
		std::cout << "Boost Exception : \n" << boost::diagnostic_information<>(e) << std::endl;
		return false;
	}
	catch(Ogre::Exception const &e)
	{
		std::cout << "Ogre Exception: \n" << e.what() << std::endl;
		return false;
	}
	catch( std::exception const &e )
	{
		std::cout << "STD Exception: \n" << e.what() << std::endl;
		return false;
	}
	catch( ... )
	{
		std::cout << "An exception of unknow type occured." << std::endl;
		return false;
	}

	return isRunning();
}

void
vl::Application::run(void)
{		
	while( isRunning() )
	{
		_mainloop(true);	
	}

	exit();
}

void
vl::Application::init(ProgramOptions const &opt)
{
	// Needs to be created here because does not currently support
	// changing the log file at run time.
	_logger = new Logger;
	_logger->setOutputFile(opt.getLogFile());

	if(!opt.show_system_console)
	{ hide_system_console(); }

	// Set the used processors
	if(opt.n_processors != -1)
	{
#ifndef _WIN32
		std::cout << "Setting number of processors is only supported on Windows." << std::endl;
#else
		HANDLE proc = ::GetCurrentProcess();
		DWORD_PTR current_mask, new_mask, system_mask;
		::GetProcessAffinityMask(proc, &current_mask, &system_mask);

		new_mask = 0;
		// @todo add checking that we don't try to use more processors than is available
		for(size_t i = opt.start_processor; i < opt.start_processor + opt.n_processors; ++i)
		{ new_mask |= 1<<i; }

		if(new_mask > current_mask)
		{ 
			std::cerr << "Couldn't change the processor affinity mask because "
				<< "there is not enough processors for the new mask." << std::endl;
		}
		else
		{
			std::clog << "Changing processor affinity mask from " << std::hex << current_mask << " to " << new_mask << std::endl;
			::SetProcessAffinityMask(proc, new_mask);
		}
#endif
	}

	_do_init(opt);
}
