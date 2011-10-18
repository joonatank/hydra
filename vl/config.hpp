/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2010-10
 *	@file config.hpp
 *
 *	This file is part of Hydra VR game engine.
 *
 */

#ifndef HYDRA_CONFIG_HPP
#define HYDRA_CONFIG_HPP

// Base class
#include "cluster/session.hpp"

// Necessary for the project settings
#include "settings.hpp"

// Necessary for the ref pointers
#include "typedefs.hpp"

#include "logger.hpp"

// Necessary for the callback structs
#include "renderer_interface.hpp"
// Necessary for callback structures
#include "cluster/server.hpp"

// Necessary for timing statistics
#include "base/timer.hpp"

namespace vl
{

class Config;

/// Callbacks
struct ConfigMsgCallback : public vl::MsgCallback
{
	ConfigMsgCallback(vl::Config *own);

	virtual ~ConfigMsgCallback(void) {}

	virtual void operator()(vl::cluster::Message const &msg);

	vl::Config *owner;
};

struct ConfigServerDataCallback : public vl::cluster::ServerDataCallback
{
	ConfigServerDataCallback(vl::Config *own);

	virtual ~ConfigServerDataCallback(void) {}

	virtual vl::cluster::Message createInitMessage(void);

	virtual vl::cluster::Message createEnvironmentMessage(void);

	virtual vl::cluster::Message createProjectMessage(void);

	/// @todo add CREATE_MSG and UPDATE_MSG also

	virtual vl::cluster::Message createResourceMessage(vl::cluster::RESOURCE_TYPE type, std::string const &name);

	vl::Config *owner;
};

/**	@class Config
 *
 */
class Config : public vl::Session
{
public:
	Config( vl::Settings const &settings,
			vl::EnvSettingsRefPtr env,
			vl::Logger &logger,
			vl::RendererUniquePtr rend );

	virtual ~Config (void);

	/// @todo this should send initialisation messages to all the rendering
	/// threads
	virtual void init( void );

	virtual void exit (void);

	virtual void render( void );

	virtual bool isRunning( void )
	{ return _running; }

	virtual void stopRunning( void )
	{ _running = false; }

	vl::GameManagerPtr getGameManager(void)
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

	// This should always be called after createMsgUpdate
	/// @todo fix const correctness, this method should not modify the object 
	vl::cluster::Message createMsgInit(void);

	vl::cluster::Message createMsgEnvironment(void) const;

	vl::cluster::Message createMsgProject(void) const;

protected :
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
	void _createResourceManager( vl::Settings const &settings, vl::EnvSettingsRefPtr env );

	void _handleMessages( void );
	void _handleMessage(vl::cluster::Message &msg);
	void _handleEventMessage(vl::cluster::Message &msg);
	void _handleCommandMessage(vl::cluster::Message &msg);

	vl::GameManagerPtr _game_manager;

	/// Settings both project and environment saved because they are requested
	/// by the slaves later than used by the Master.
	/// Current project settings
	vl::Settings _proj;
	/// Current environment settings
	vl::EnvSettingsRefPtr _env;

	vl::cluster::ServerRefPtr _server;
	/// timer used to see how much delay there is with server updates
	vl::timer _server_timer;

	vl::timer _stats_timer;

	bool _running;

	// Renderer
	RendererUniquePtr _renderer;

	// Update messages for this frame
	vl::cluster::Message _msg_create;
	vl::cluster::Message _msg_update;

	// callback provided messages
	std::deque<vl::cluster::Message> _messages;

	// Callbacks owned by us
	std::vector<vl::Callback *> _callbacks;

	/// receivers for logging
	/// first is the last logged message, second is the receiver it self
	//std::vector<std::pair<uint32_t, vl::LogReceiver *> _log_receivers;

};	// class Config

}	// namespace vl

#endif	// HYDRA_CONFIG_HPP
