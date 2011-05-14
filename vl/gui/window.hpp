/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-02
 *	@file window.hpp
 *
 */

#ifndef VL_GUI_WINDOW_HPP
#define VL_GUI_WINDOW_HPP

#include <CEGUI/CEGUIWindow.h>

#include <string>
#include <deque>

// Necessary for log level
#include "logger.hpp"

#include "typedefs.hpp"

namespace vl
{

namespace gui
{
/** @class Window
 *	@brief Wrapper around CEGUI Window, provides the callbacks
 */
class Window
{
public :
	Window(CEGUI::Window *win, vl::gui::GUI *creator)
		: _window(win), _creator(creator)
	{
		assert(_window);
		assert(_creator);
	}

	virtual ~Window(void)
	{
		// Should not delete the CEGUI window, or if it does it needs to
		// use CEGUI functions for that.
	}

	void setVisible(bool visible)
	{
		_window->setVisible(visible);
	}

protected :
	CEGUI::Window *_window;
	vl::gui::GUI *_creator;
};

class ConsoleWindow : public Window
{
public :
	ConsoleWindow(CEGUI::Window *win, vl::gui::GUI *creator);

	~ConsoleWindow(void);

	void printTo(std::string const &text, double time,
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

	bool wantsLogging(void) const
	{ return true; }

private :
	std::deque<std::string> _console_memory;
	int _console_memory_index;
	std::string _console_last_command;

};

class EditorWindow : public Window
{
public :
	EditorWindow(CEGUI::Window *win, vl::gui::GUI *creator);

	~EditorWindow(void);

	/// GECUI callbacks

	/// Menubar buttons clicked
	bool onNewClicked( CEGUI::EventArgs const &e );
	bool onOpenClicked( CEGUI::EventArgs const &e );
	bool onSaveClicked( CEGUI::EventArgs const &e );
	bool onQuitClicked( CEGUI::EventArgs const &e );
	bool onResetClicked( CEGUI::EventArgs const &e );
	bool onImportSceneClicked( CEGUI::EventArgs const &e );
	bool onReloadScenesClicked( CEGUI::EventArgs const &e );
	bool onNewScriptClicked( CEGUI::EventArgs const &e );
	bool onAddScriptClicked( CEGUI::EventArgs const &e );
	bool onReloadScriptsClicked( CEGUI::EventArgs const &e );

	/// Checkboxes
	bool onShowAxisChanged( CEGUI::EventArgs const &e );
	bool onShowNamesChanged( CEGUI::EventArgs const &e );
	bool onShowJointsChanged( CEGUI::EventArgs const &e );

};

}	// namespace gui

}	// namespace vl

#endif	// VL_GUI_WINDOW_HPP
