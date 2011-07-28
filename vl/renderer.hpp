/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-01
 *	@file renderer.hpp
 *
 */

#ifndef VL_RENDERER_HPP
#define VL_RENDERER_HPP

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
class Renderer : public vl::RendererInterface, public Session
{
public :
	Renderer(std::string const &name);

	virtual ~Renderer(void);

	void init(vl::EnvSettingsRefPtr env);

	vl::EnvSettingsRefPtr getEnvironment(void)
	{ return _env; }

	virtual void setMeshManager(vl::MeshManagerRefPtr mesh_man)
	{ _mesh_manager = mesh_man; }

	vl::EnvSettings::Node getNodeConf( void );

	vl::EnvSettings::Window getWindowConf( std::string const &window_name );

	vl::ogre::RootRefPtr getRoot( void )
	{ return _root; }

	Ogre::SceneManager *getSceneManager( void )
	{ return _ogre_sm; }

	vl::CameraPtr getCamera( void )
	{ return _camera; }

	vl::Player const &getPlayer( void ) const
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

	/// Overrides from Abstract interface
	/// @brief interface through which the Renderer is controlled
// 	virtual void handleMessage(vl::cluster::Message &msg);

	virtual void setProject(vl::cluster::Message &msg);
	
	virtual void initScene(vl::cluster::Message &msg);
	
	virtual void updateScene(vl::cluster::Message &msg);
	
	virtual void createSceneObjects(vl::cluster::Message &msg);
	
	virtual void print(vl::cluster::Message &msg);

	virtual void setSendMessageCB(vl::MsgCallback *cb);

	/// Log Receiver overrides
	virtual bool logEnabled(void) const;

	virtual void logMessage(LogMessage const &msg);

	virtual uint32_t nLoggedMessages(void) const;

protected :

	/// Ogre helpers
	void _createOgre(vl::EnvSettingsRefPtr env);
	void _initialiseResources(vl::Settings const &set);
	Ogre::SceneManager *_createOgreSceneManager(vl::ogre::RootRefPtr root, std::string const &name);

	/// Distribution helpers
	/// Syncs to the master copy stored when an Update message was received
	void _syncData( void );
	/// Updates command data, uses versioning to and indexes to provide an
	/// Commands that can be sent from Master thread
	void _updateDistribData( void );

	/// Input events
	void _sendEvents( void );

	void _createWindow( vl::EnvSettings::Window const &winConf );

	/**	@todo should write the screenshot to the project directory not
	 *	to current directory
	 *	Add the screenshot dir to DistributedSettings
	 *	@todo the format of the screenshot name should be
	 *	screenshot_{project_name}-{year}-{month}-{day}-{time}-{window_name}.png
	 */
	void _takeScreenshot( void );

	std::string _name;

	/// EnvSettings mapped from Master
	vl::EnvSettingsRefPtr _env;
	vl::Settings _settings;

	vl::MeshManagerRefPtr _mesh_manager;

	/// Ogre data
	vl::ogre::RootRefPtr _root;
	Ogre::SceneManager *_ogre_sm;

	vl::CameraPtr _camera;

	/// Distributed data
	vl::SceneManagerPtr _scene_manager;
	vl::Player *_player;
	std::string _active_camera_name;
	uint32_t _screenshot_num;

	/// Input events to be sent
	std::vector<vl::cluster::EventData> _events;

	std::vector<vl::Window *> _windows;

	/// GUI related
	vl::gui::GUIRefPtr _gui;

	/// ID list of objects which should be ignored when updating
	/// used for GUI at the moment.
	std::vector<uint32_t> _ignored_distributed_objects;
	
	bool _running;
	bool _rendering;

	std::vector<vl::cluster::ObjectData> _objects;

	// Callbacks
	vl::MsgCallback *_send_message_cb;

	// LogReceiver
	uint32_t _n_log_messages;

};	// class Renderer

}	// namespace vl

#endif // VL_RENDERER_HPP
