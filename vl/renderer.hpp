/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2013 Savant Simulators
 *
*	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-01
 *	@file renderer.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

#ifndef HYDRA_RENDERER_HPP
#define HYDRA_RENDERER_HPP

// Necessary for HYDRA_API
#include "defines.hpp"

/// Owned objects
#include "ogre_root.hpp"
#include "player.hpp"
#include "scene_manager.hpp"
#include "window_interface.hpp"

#include "settings.hpp"
// Necessary for callbacks
#include <boost/signal.hpp>
// Base class
#include "logger.hpp"

namespace vl
{

/**	@class Renderer
 *	@brief Representation of a pipe i.e. a single GPU
 *
 *	A Pipe is always run on single GPU.
 *	Pipe objects are executed completely in a separate threads from each other
 *	and from the application thread.
 *	At the moment the application supports only a single Pipe objects because
 *	of the limitations in Ogre Design.
 *
 *	@todo should be distributed
 *	Master owns N+1 Renderers/Pipes where N is the number of Slaves
 *	- Master updates all Renderers and all Windows/Channels in them
 *	- Distributed to slaves (all Renderers/Windows/Channels to all slaves for now)
 *	- Update on slaves
 *	- Render
 *	Extension to this would be to add RPC to Renderer so we can use it for
 *	rendering commands (draw, swap, capture).
 *	Proper RPC framework would reduce problems in the rendering pipeline
 *	caused by the horrible message system (with switches etc.)
 */
class HYDRA_API Renderer : public LogReceiver
{
	// signals
	typedef boost::signal<void (std::string const &)> CommandSent;
	typedef boost::signal<void (vl::cluster::EventData const &)> EventSent;

public :
	Renderer(Session *session, std::string const &name);

	virtual ~Renderer(void);

	void init(void);

	/// Necessary for creating Ogre windows in Window
	/// We could also have create window method here but that's confusing
	/// We could also create Ogre windows here and pass them to Window
	vl::ogre::RootRefPtr getRoot( void )
	{ return _root; }

	vl::Player *getPlayer(void)
	{ return _player; }
	vl::Player const *getPlayer(void) const
	{ return _player; }

	// Used by client for setting up Mesh loaded callbacks
	vl::MeshManagerRefPtr getMeshManager(void)
	{ return _mesh_manager; }

	/// Can not be owned by Renderer because GameManager needs to create it
	/// Not a good way to set it though, we should pass it to constructor
	void setMeshManager(vl::MeshManagerRefPtr mesh_man)
	{ _mesh_manager = mesh_man; }

	/// Messaging system
	void sendEvent( vl::cluster::EventData const &event );

	void sendCommand( std::string const &cmd );

	/// Rendering functions
	void capture(void);
	
	void draw( void );
	
	void swap( void );

	std::string const &getName( void ) const
	{ return _name; }

	bool guiShown(void) const;

	/// @brief passes the messages to GUI::Console
	void printToConsole(std::string const &text, double time,
						std::string const &type = std::string(),
						vl::LOG_MESSAGE_LEVEL lvl = vl::LML_NORMAL);

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
	void setProject(vl::Settings const &settings);
	
	/// @brief clears project settings
	void clearProject(void);

	/// @todo everything with a Message in it should be moved to Slave
	/// Slave handles the translation from Message to discrete commands
	/// and commands the Renderer (same with Master)
	/// At later point we might even combine some of the functionality
	/// into Application and use pure virtual for Message sending/receiving
	void updateScene(vl::cluster::Message &msg);
	
	void createSceneObjects(vl::cluster::Message &msg);

	void addCommandListener(CommandSent::slot_type const &slot)
	{ _command_signal.connect(slot); }

	void addEventListener(EventSent::slot_type const &slot)
	{ _event_signal.connect(slot); }

	gui::GUIRefPtr getGui(void)
	{ return _gui; }

	vl::Session *getSession(void)
	{ return _session; }

	vl::Pipe *getPipe(void)
	{ return _pipe; }

	/// @brief Helper function for creating GUI when window is initalised
	/// should not be called elsewhere than Window and GUI creation
	/// NOP if GUI has already been initialised
	/// @return true if initialised, false otherwise
	bool _initialiseGUI(void);

	/// Hack for getting current view and projection matrices
	/// This is a hack because these don't take into account
	/// multiple windows and multiple channels it's also useless
	/// on slaves because these are provided mainly for python interface.
	///
	/// Proper solution to the problem would be to provide interface for
	/// both Windows and Channels to python and
	/// change Renderer, Window and Channel to be shared.
	/// Would need modify the Renderer to be a shared Node, so master would own
	/// N+1 of them where N is the number of slaves.
	Ogre::Matrix4 const &getViewMatrix(void) const
	{ return _view_matrix; }
	Ogre::Matrix4 const &getProjectionMatrix(void) const
	{ return _proj_matrix; }

	/// Log Receiver overrides
	virtual bool logEnabled(void) const;

	virtual void logMessage(LogMessage const &msg);

	virtual uint32_t nLoggedMessages(void) const;

protected :

	/// Distribution helpers
	/// Syncs to the master copy stored when an Update message was received
	void _syncData( void );
	/// Updates command data, uses versioning to and indexes to provide an
	/// Commands that can be sent from Master thread
	void _updateDistribData( void );

	typedef std::map<uint64_t, OBJ_TYPE> IdTypeMap;
	void _create_objects(IdTypeMap const &objects, IdTypeMap &left_overs);
	IdTypeMap _objects_not_yet_created;

	/**	@todo should write the screenshot to the project directory not
	 *	to current directory
	 *	Add the screenshot dir to DistributedSettings
	 *	@todo the format of the screenshot name should be
	 *	screenshot_{project_name}-{year}-{month}-{day}-{time}-{window_name}.png
	 */
	void _takeScreenshot( void );

	void _check_materials(uint64_t const id);

	std::string _name;

	vl::Session *_session;
	/// Pipe for this Renderer
	vl::Pipe *_pipe;
	/// We store all the pipes received from Master because we can't access
	/// their names before they are completely serialised.
	std::vector<vl::Pipe *> _all_pipes;

	vl::MeshManagerRefPtr _mesh_manager;
	vl::MaterialManagerRefPtr _material_manager;

	/// Ogre data
	vl::ogre::RootRefPtr _root;

	/// Distributed data
	vl::SceneManagerPtr _scene_manager;
	vl::Player *_player;
	uint32_t _screenshot_num;

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

	// Projection and View matrices used for last frame
	// Used for the python interface.
	Ogre::Matrix4 _view_matrix;
	Ogre::Matrix4 _proj_matrix;

};	// class Renderer

}	// namespace vl

#endif // HYDRA_RENDERER_HPP
