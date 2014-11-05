/**
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-12
 *	@file slave.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

#include "slave.hpp"

#include "cluster/client.hpp"

#include "base/sleep.hpp"

#include "base/exceptions.hpp"

// Necessary for creating Renderer for slave and master
#include "renderer.hpp"

vl::Slave::Slave(void)
	: Application()
{}

void
vl::Slave::injectCommand(std::string const &cmd)
{
	assert(_slave_client);

	vl::cluster::Message msg( vl::cluster::MSG_COMMAND, 0, vl::time() );
	// Write size and string and terminating character
	msg.write( cmd.size()+1 );
	msg.write( cmd.c_str(), cmd.size()+1 );

	_slave_client->sendMessage(msg);
}

void
vl::Slave::injectEvent(vl::cluster::EventData const &event)
{
		// Add to event stack for sending them at once in one message to the Master
	_events.push_back(event);
}

void
vl::Slave::exit(void)
{
}

bool
vl::Slave::isRunning(void) const
{
	return _slave_client->isRunning();
}

/// -------------------- Private virtual overrides ---------------------------
void
vl::Slave::_mainloop(bool sleep)
{
	if( !_events.empty() )
	{
		// @todo fix the time and frame number
		vl::cluster::Message msg(vl::cluster::MSG_INPUT, 0, vl::time());
		std::vector<vl::cluster::EventData>::iterator iter;
		for( iter = _events.begin(); iter != _events.end(); ++iter )
		{
			iter->copyToMessage(&msg);
		}
		_events.clear();
		_slave_client->sendMessage(msg);
	}

	// run main loop
	_slave_client->mainloop();

	/// @todo test
	/// Windows can have problems with context switching.
	/// At least this is the case for Windows XP.
	/// Might need a workaround for some or all Windows versions
	/// For now use WIN_ZERO_SLEEP define for testing.
	/// Real solution will need a separate busy-wait while rendering 
	/// and context switching while not.
	///
	/// Linux can not handle busy wait,
	/// much faster with context switching in every iteration.
	vl::msleep(uint32_t(0));
}

void
vl::Slave::_do_init(ProgramOptions const &opt)
{
	if(opt.slave_name.empty())
	{
		assert(false);
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Empty Slave name not allowed"));
	}

	std::cout << "vl::Slave : name = " << opt.slave_name << std::endl;

	// We should hand over the Renderer to either client or config
	_renderer.reset( new Renderer(this, opt.slave_name) );

	_renderer->addCommandListener(boost::bind(&Slave::injectCommand, this, _1));
	_renderer->addEventListener(boost::bind(&Slave::injectEvent, this, _1));

	std::string hostname = opt.server_hostname;
	uint16_t port = opt.server_port;
	_slave_client.reset(new vl::cluster::Client(hostname.c_str(), port, _renderer));
}
