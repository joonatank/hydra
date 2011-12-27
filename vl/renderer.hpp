/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011/10 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-01
 *	@file renderer.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef HYDRA_RENDERER_HPP
#define HYDRA_RENDERER_HPP

// Necessary for HYDRA_API
#include "defines.hpp"

#include "ogre_root.hpp"
#include "base/envsettings.hpp"
#include "player.hpp"
#include "scene_manager.hpp"

#include "settings.hpp"

#include "logger.hpp"

// Base class
#include "renderer_interface.hpp"

namespace vl
{

/**	@class Pipe
 *	@brief Representation of a pipe i.e. a single GPU
 *
 *	A Pipe is always run on single GPU.
 *	Pipe objects are executed completely in a separate threads from each other
 *	and from the application thread.
 *	At the moment the application supports only a single Pipe objects because
 *	of the limitations in Ogre Design.
 */
class HYDRA_API Renderer : public vl::RendererInterface, public Session
{
public :
	Renderer(std::string const &name);

	virtual ~Renderer(void);

	void init(vl::config::EnvSettingsRefPtr env);

	vl::config::EnvSettingsRefPtr getEnvironment(void)
	{ return _env; }

	virtual void setMeshManager(vl::MeshManagerRefPtr mesh_man)
	{ _mesh_manager = mesh_man; }

	vl::config::Node const &getNodeConf(void) const;

	vl::config::Window const &getWindowConf(std::string const &window_name) const;

	vl::ogre::RootRefPtr getRoot( void )
	{ return _root; }

	Ogre::SceneManager *getSceneManager( void )
	{ return _ogre_sm; }

	vl::Player const &getPlayer( void ) const
	{ return *_player; }

	vl::Player const &getPlayer(void)
	{ return *_player; }

	virtual vl::MeshManagerRefPtr getMeshManager(void)
	{ return _mesh_manager; }

	/// Messaging system
	void sendEvent( vl::cluster::EventData const &event );

	void sendCommand( std::string const &cmd );

	/// RenderingInterface overrides
	/// Loop functions
	virtual void capture(void);
	
	virtual void draw( void );
	
	virtual void swap( void );

	std::string const &getName( void ) const
	{ return _name; }

	typedef std::vector<vl::ProjSettingsRefPtr> ProjectList;

	virtual bool guiShown(void) const;

	/// @brief passes the messages to GUI::Console
	void printToConsole(std::string const &text, double time,
						std::string const &type = std::string(),
						vl::LOG_MESSAGE_LEVEL lvl = vl::LML_NORMAL);

	void reloadProjects( vl::Settings const &set );

	/** @todo
	 * Problematic because the Project config should be
	 * updatable during the application run
	 * And this one will create them anew, so that we need to invalidate
	 * the scene and reload everything
	 * NOTE
	 * Combining the project configurations is not done automatically
	 * so they either need special structure or we need to iterate over
	 * all of them always.
	 */
	virtual void setProject(vl::Settings const &settings);
	
	/// @todo everything with a Message in it should be moved to Slave
	/// Slave handles the translation from Message to discrete commands
	/// and commands the Renderer (same with Master)
	/// At later point we might even combine some of the functionality
	/// into Application and use pure virtual for Message sending/receiving
	virtual void initScene(vl::cluster::Message &msg);
	
	virtual void updateScene(vl::cluster::Message &msg);
	
	virtual void createSceneObjects(vl::cluster::Message &msg);
	
	virtual void print(vl::cluster::Message &msg);

	virtual void addCommandListener(CommandSent::slot_type const &slot)
	{ _command_signal.connect(slot); }

	virtual void addEventListener(EventSent::slot_type const &slot)
	{ _event_signal.connect(slot); }

	/// Log Receiver overrides
	virtual bool logEnabled(void) const;

	virtual void logMessage(LogMessage const &msg);

	virtual uint32_t nLoggedMessages(void) const;

	vl::IWindow *createWindow(vl::config::Window const &winConf);

protected :

	/// Ogre helpers
	void _createOgre(vl::config::EnvSettingsRefPtr env);

	Ogre::SceneManager *_createOgreSceneManager(vl::ogre::RootRefPtr root, std::string const &name);

	/// Distribution helpers
	/// Syncs to the master copy stored when an Update message was received
	void _syncData( void );
	/// Updates command data, uses versioning to and indexes to provide an
	/// Commands that can be sent from Master thread
	void _updateDistribData( void );

	/**	@todo should write the screenshot to the project directory not
	 *	to current directory
	 *	Add the screenshot dir to DistributedSettings
	 *	@todo the format of the screenshot name should be
	 *	screenshot_{project_name}-{year}-{month}-{day}-{time}-{window_name}.png
	 */
	void _takeScreenshot( void );

	void _check_materials(uint64_t const id);

	std::string _name;

	/// EnvSettings mapped from Master
	vl::config::EnvSettingsRefPtr _env;
	vl::Settings _settings;

	vl::MeshManagerRefPtr _mesh_manager;
	vl::MaterialManagerRefPtr _material_manager;

	/// Ogre data
	vl::ogre::RootRefPtr _root;
	Ogre::SceneManager *_ogre_sm;

	/// Distributed data
	vl::SceneManagerPtr _scene_manager;
	vl::Player *_player;
	uint32_t _screenshot_num;

	std::vector<vl::Window *> _windows;

	/// GUI related
	vl::gui::GUIRefPtr _gui;

	/// ID list of objects which should be ignored when updating
	/// used for GUI at the moment.
	std::vector<uint32_t> _ignored_distributed_objects;
	
	bool _running;
	bool _rendering;

	std::vector<vl::cluster::ObjectData> _objects;

	// Signals
	CommandSent _command_signal;
	EventSent _event_signal;

	// LogReceiver
	uint32_t _n_log_messages;

	std::vector<vl::MaterialRefPtr> _materials_to_check;

};	// class Renderer

}	// namespace vl

#endif // HYDRA_RENDERER_HPP
