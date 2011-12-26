/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-12
 *	@file slave.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 */

#include "slave.hpp"

#include "cluster/client.hpp"

#include "base/sleep.hpp"

#include "base/envsettings.hpp"
// Necessary for creating Renderer for slave and master
#include "renderer.hpp"

vl::Slave::Slave(void)
	: Application()
{}

bool
vl::Slave::isRunning(void) const
{
	return _slave_client->isRunning();
}


vl::RendererInterface *
vl::Slave::getRenderer(void) const
{
	return _renderer.get();
}

vl::GameManagerPtr
vl::Slave::getGameManager(void) const
{
	return 0;
}


/// -------------------- Private virtual overrides ---------------------------
void
vl::Slave::_mainloop(bool sleep)
{
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
	vl::msleep(0);
}

void
vl::Slave::_exit(void)
{
}

void
vl::Slave::_do_init(vl::config::EnvSettingsRefPtr env, ProgramOptions const &opt)
{
	assert(env->isSlave());

	/// Correct name has been set
	std::cout << "vl::Application::Application : name = " << env->getName() << std::endl;

	// We should hand over the Renderer to either client or config
	_renderer.reset( new Renderer(env->getName()) );

	char const *host = env->getServer().hostname.c_str();
	uint16_t port = env->getServer().port;
	_slave_client.reset(new vl::cluster::Client(host, port, _renderer));
}
