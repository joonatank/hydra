/**
 *	Copyright (c) 2010 - 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-10
 *	@file master.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */


#ifndef HYDRA_MASTER_HPP
#define HYDRA_MASTER_HPP

// Base classes
#include "cluster/session.hpp"
#include "application.hpp"

// Necessary for the project settings
#include "settings.hpp"
// Necessary for the ref pointers
#include "typedefs.hpp"
// Necessary for callback structures (RequestedMessage)
#include "cluster/server.hpp"
// Necessary for timing statistics
#include "base/chrono.hpp"

namespace vl
{

vl::config::EnvSettingsRefPtr getMasterSettings( vl::ProgramOptions const & options );

/**	@class Master
 *
 */
class HYDRA_API Master : public vl::Session, public vl::Application
{
public:
	Master(void);

	virtual ~Master(void);

	/// @todo this should send initialisation messages to all the rendering
	/// threads
	virtual void init(std::string const &global_file,
			std::string const &project_file);

	virtual void exit (void);

	virtual void render( void );

	bool isRunning(void) const
	{ return _running; }

	/// Used internally for some reason
	vl::GameManagerPtr getGameManager(void) const
	{ return _game_manager; }

	/// Message callback system functions
	bool messages(void) const
	{ return _messages.size(); }

	vl::cluster::Message popMessage(void);

	/// Push new message from callback to the stack
	void pushMessage(vl::cluster::Message const &msg);

	/// Callback methods called from Server to retrieve the messages needed
	/// by the Rendering clients.
	/// Which message is needed depends on the client state. That's why these
	/// are implemented as callbacks.
	/// @todo these can be moved to private as we are using requests now
	vl::cluster::Message createMsgInit(void) const;

	vl::cluster::Message createResourceMessage(
			vl::cluster::RESOURCE_TYPE type, std::string const &name) const;

	void messageRequested(vl::cluster::RequestedMessage const &);

	void injectEvent(vl::cluster::EventData const &);

	// Callback for Project settings changes
	void settingsChanged(void);
	
	// Callback for clearing the project
	void clearProjectCallback(void);

	void quit_callback(void)
	{
		std::clog << "vl::Master : Quit callback called." << std::endl;
		_running = false; 
	}

	vl::time getSimulationTime(void) const;

/// Private virtual overrides from Application
private :
	virtual void _mainloop(bool sleep);

	virtual void _do_init(ProgramOptions const &opt);

protected :
	/// Parse the Environment file and create the approriate Pipes
	/// One pipe per Renderer
	/// Pipes need to be properly named so even when we broadcast all
	/// of them Renderers can pick the proper pipe for themselves.
	void _createWindows(vl::config::EnvSettingsRefPtr env);

	// Cluster rpc calss
	void _updateServer( void );
	void _updateRenderer(void);

	// Updates the messages stored per frame
	// This should definitely not be called more than once per frame
	void _updateFrameMsgs(void);
	// This shouldn't be called more than once per frame, resets changes
	void _createMsgCreate(void);
	// This shouldn't be called more than once per frame, resets changes
	void _createMsgUpdate(void);

	/// Resources
	void _createResourceManager(vl::Settings const &settings, vl::config::EnvSettingsRefPtr env);

	void _handleMessages( void );
	void _handleMessage(vl::cluster::Message &msg);
	void _handleEventMessage(vl::cluster::Message &msg);
	// @todo should use a custom data structure that is not tied to the cluster
	// also no handle necessary here, move to EventManager after implementing
	// custom type
	void _handleEvent(vl::cluster::EventData &evt);
	void _handleCommandMessage(vl::cluster::Message &msg);

	vl::GameManagerPtr _game_manager;

	vl::cluster::ServerRefPtr _server;
	// @todo we should probably have a reference pointer here
	RendererPtr _renderer;
	std::vector<PipeRefPtr> _pipes;

	/// Settings both project and environment saved because they are requested
	/// by the slaves later than used by the Master.
	/// Current project settings
	vl::Settings _proj;
	/// Current environment settings
	vl::config::EnvSettingsRefPtr _env;

	/// timer used to see how much delay there is with server updates
	vl::chrono _server_timer;

	vl::chrono _stats_timer;

	bool _running;

	uint32_t _frame;
	vl::chrono _sim_timer;

	// Update messages for this frame
	vl::cluster::Message _msg_create;
	vl::cluster::Message _msg_update;

	// callback provided messages
	std::deque<vl::cluster::Message> _messages;

	std::vector<uint32_t> _spawned_processes;

};	// class Master

}	// namespace vl

#endif	// HYDRA_MASTER_HPP
