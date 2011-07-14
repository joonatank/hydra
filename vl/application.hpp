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

// Necessary for Ref ptrs
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
	/** @brief start the rendering threads and initialise the application
	 *	@param env the environment settings to use
	 *	@param settings the project settings to use
	 *	@param logger the log manager instance to use
	 *	@param auto_fork wether to autolaunch slaves or not
	 * 
	 *	Will create a rendering thread and start it running.
	 *	If this is the master node will also initialise the master node
	 *	and send messages to the rendering threads.
	 */
	Application(vl::EnvSettingsRefPtr env, vl::Settings const &settings, vl::Logger &logger, bool auto_fork);

	virtual ~Application( void );

	void run( void );

protected:

	void _render( uint32_t const frame );

	vl::ConfigRefPtr _master;

	vl::cluster::ClientRefPtr _slave_client;

};	// class Application

}	// namespace vl

#endif // VL_APPLICATION_HPP
