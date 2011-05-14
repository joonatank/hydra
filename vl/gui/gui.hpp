/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-02
 *	@file gui.hpp
 *
 *	This file is part of Hydra VR game engine.
 */

#ifndef HYDRA_GUI_GUI_HPP
#define HYDRA_GUI_GUI_HPP

#include "distributed.hpp"
#include "session.hpp"

// Necessary for command callback
#include "renderer_interface.hpp"

namespace vl
{

namespace gui
{

/**	@class GUI
 *	@brief Distributed class for GUI modifications
 */
class GUI : public vl::Distributed
{
public :
	/// @brief Master constructor
	GUI(vl::Session *session);

	/// @brief Slave or Renderer constructor
	/// @param session the distributed session this GUI is attached to
	/// @param id ID of this GUI, has to be valid
	/// @param cb callback for sendCommand, ownership is passed to this
	GUI(vl::Session *session, uint64_t id, vl::CommandCallback *cb);
	
	void initGUI(vl::Window *win);
	void initGUIResources(vl::Settings const &set);
	void addGUIResourceGroup(std::string const &name, fs::path const &path);
	void createGUI(void);

	EditorWindowRefPtr getEditor(void)
	{ return _editor; }

	void showEditor(void)
	{ setEditorVisibility(true); }

	void hideEditor(void)
	{ setEditorVisibility(false); }

	void setEditorVisibility(bool vis);

	bool editorShown( void ) const
	{ return _editor_shown; }

	ConsoleWindowRefPtr getConsole(void)
	{ return _console; }

	void setConsoleVisibility(bool vis);

	void showConsole(void)
	{ setConsoleVisibility(true); }

	void hideConsole(void)
	{ setConsoleVisibility(false); }

	bool consoleShown( void ) const
	{ return _console_shown; }

	WindowRefPtr getStats(void)
	{ return _stats; }

	void setStatsVisibility(bool vis);

	void showStats(void)
	{ setStatsVisibility(true); }

	void hideStats(void)
	{ setStatsVisibility(false); }

	bool statsShown( void ) const
	{ return _stats_shown; }

	WindowRefPtr getLoadingScreen(void)
	{ return _loading_screen; }

	void setLoadingScreenVisibility(bool vis);

	void showLoadingScreen(void)
	{ setLoadingScreenVisibility(true); }

	void hideLoadingScreen(void)
	{ setLoadingScreenVisibility(false); }

	bool loadingScreenShown( void ) const
	{ return _loading_screen_shown; }

	bool shown( void ) const
	{ return consoleShown() || editorShown(); }

	void sendCommand(std::string const &cmd);

	enum DirtyBits
	{
		DIRTY_EDITOR = Distributed::DIRTY_CUSTOM << 0,
		DIRTY_CONSOLE = Distributed::DIRTY_CUSTOM << 1,
		DIRTY_STATS = Distributed::DIRTY_CUSTOM << 2,
		DIRTY_LOADING_SCREEN = Distributed::DIRTY_CUSTOM << 3,
		DIRTY_CUSTOM = Distributed::DIRTY_CUSTOM << 4,
	};

/// Private Methods
private :

	virtual void serialize( cluster::ByteStream &msg, const uint64_t dirtyBits );

	virtual void deserialize( cluster::ByteStream &msg, const uint64_t dirtyBits );

/// Data
private :
	bool _editor_shown;
	bool _console_shown;
	bool _stats_shown;
	bool _loading_screen_shown;

	vl::gui::EditorWindowRefPtr _editor;
	vl::gui::ConsoleWindowRefPtr _console;
	vl::gui::WindowRefPtr _stats;
	vl::gui::WindowRefPtr _loading_screen;

	vl::CommandCallback *_cmd_cb;

};	// class GUI

}	// namespace gui

}	// namespace vl

#endif	// HYDRA_GUI_GUI_HPP
