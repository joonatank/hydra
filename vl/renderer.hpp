/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-01
 *	@file renderer.hpp
 *
 */

#ifndef VL_RENDERER_HPP
#define VL_RENDERER_HPP

#include "eq_ogre/ogre_root.hpp"
#include "base/envsettings.hpp"
#include "player.hpp"
#include "scene_manager.hpp"

#include "settings.hpp"
#include "gui/gui.hpp"

#include "logger.hpp"

#include <CEGUI/CEGUIWindow.h>

// Base class
#include "renderer_interface.hpp"

namespace vl
{

class Window;

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

	vl::Settings const &getProject(void) const
	{ return _settings; }

	vl::EnvSettings::Node getNodeConf( void );

	vl::EnvSettings::Window getWindowConf( std::string const &window_name );

	vl::ogre::RootRefPtr getRoot( void )
	{ return _root; }

	Ogre::SceneManager *getSceneManager( void )
	{ return _ogre_sm; }

	Ogre::Camera *getCamera( void )
	{ return _camera; }

	vl::Player const &getPlayer( void ) const
	{ return *_player; }

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

	virtual bool guiShown( void ) const
	{
		if( !_gui )
		{ return false; }
		return _gui->shown();
	}

	CEGUI::Window *getEditor( void )
	{ return _editor; }

	CEGUI::Window *getConsole( void )
	{ return _console; }

	void printToConsole(std::string const &text, double time,
						std::string const &type = std::string(),
						vl::LOG_MESSAGE_LEVEL lvl = vl::LML_NORMAL);

	/// GECUI callbacks
	/// Console events
	bool onConsoleInputAccepted(CEGUI::EventArgs const &e);

	/// @brief Scroll the console memory using up and down arrows
	/// If there is new user input it will be saved to the bottom of scroll
	bool onConsoleInputKeyDown(CEGUI::EventArgs const &e);

	/// @brief When console is shown it will automatically focus on the input
	bool onConsoleShow(CEGUI::EventArgs const &e);

	/// Reload the projects
	void reloadProjects( vl::Settings const &set );

	/// GUI specific
	void initGUIResources( vl::Settings const &set );
	void addGUIResourceGroup( std::string const &name, fs::path const &path );
	void createGUI( void );

	/// Overrides from Abstract interface
	/// @brief interface through which the Renderer is controlled
	virtual void handleMessage(vl::cluster::Message &msg);

	virtual void setSendMessageCB(vl::MsgCallback *cb);

	/// Log Receiver overrides
	virtual bool logEnabled(void) const;

	virtual void logMessage(LogMessage const &msg);

	virtual uint32_t nLoggedMessages(void) const;

protected :

	// Object creations
	void guiCreated(vl::gui::GUI *gui);
	void playerCreated(vl::Player *player);
	void sceneManagerCreated(vl::SceneManager *sm);

	/// GUI specific
	void _initGUI( void );

	/// Ogre helpers
	void _createOgre(vl::EnvSettingsRefPtr env);
	void _loadScene( vl::ProjSettings::Scene const &scene );
	void _setCamera( void );

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

	/// message passing
	void _handleCreateMsg(vl::cluster::Message &msg);
	void _handleUpdateMsg(vl::cluster::Message &msg);
	void _handlePrintMsg(vl::cluster::Message &msg);

	std::string _name;

	/// EnvSettings mapped from Master
	vl::EnvSettingsRefPtr _env;
	/// ProjectSettings mapped from Master
	vl::Settings _settings;

	/// Ogre data
	vl::ogre::RootRefPtr _root;
	Ogre::SceneManager *_ogre_sm;
	Ogre::Camera *_camera;

	/// Distributed data
	vl::SceneManagerPtr _scene_manager;
	vl::Player *_player;
	std::string _active_camera_name;
	uint32_t _screenshot_num;

	/// Input events to be sent
	std::vector<vl::cluster::EventData> _events;

	std::vector<vl::Window *> _windows;

	/// GUI related
	vl::gui::GUI *_gui;

	CEGUI::Window *_console;
	CEGUI::Window *_editor;
	CEGUI::Window *_loading_screen;
	CEGUI::Window *_stats;

	std::deque<std::string> _console_memory;
	int _console_memory_index;
	std::string _console_last_command;

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
