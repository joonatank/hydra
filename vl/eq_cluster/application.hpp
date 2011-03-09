/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2010-12
 *
 *	Program starting point
 *	@todo should probably be replaced with just the main function
 *	or merge Config and Application classes and create the threads in the main
 */

#ifndef VL_APPLICATION_HPP
#define VL_APPLICATION_HPP

#include <boost/thread.hpp>

#include <stdint.h>

// Necessary for EnvSettingsRefPtr
#include "typedefs.hpp"
// Necessary for vl::ProgramOptions
#include "program_options.hpp"
#include "logger.hpp"

namespace vl
{

vl::EnvSettingsRefPtr getMasterSettings( vl::ProgramOptions const & options );
vl::Settings getProjectSettings( vl::ProgramOptions const &options );
vl::EnvSettingsRefPtr getSlaveSettings( vl::ProgramOptions const &options );

class Config;

class Application
{
public:
	/// @brief start the rendering threads and initialise the application
	///
	/// Will create a rendering thread and start it running.
	/// If this is the master node will also initialise the master node
	/// and send messages to the rendering threads.
	Application( vl::EnvSettingsRefPtr env, vl::Settings const &, vl::Logger &logger );

	virtual ~Application( void );

	void run( void );

protected:

	void _render( uint32_t const frame );

	vl::Config *_config;

	boost::thread *_pipe_thread;

};	// class Client

}	// namespace vl

#endif // VL_APPLICATION_HPP
