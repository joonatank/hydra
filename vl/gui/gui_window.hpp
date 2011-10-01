/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-02
 *	@file GUI/gui_window.hpp
 *	
 *	This file is part of Hydra VR game engine.
 */

#ifndef HYDRA_GUI_WINDOW_HPP
#define HYDRA_GUI_WINDOW_HPP

#include <CEGUI/CEGUIWindow.h>

#include <string>
#include <deque>

// Necessary for distribution
#include "distributed.hpp"
#include "session.hpp"

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
class Window : public vl::Distributed
{
public :

	Window(vl::gui::GUI *creator, std::string const &layout = std::string());

	virtual ~Window(void)
	{
		// Should not delete the CEGUI window, or if it does it needs to
		// use CEGUI functions for that.
	}

	bool isVisible(void) const
	{ return _visible; }

	void setVisible(bool visible);

	void toggleVisible(void)
	{ setVisible(!isVisible()); }

	void hide(void)
	{ setVisible(false); }

	void show(void)
	{ setVisible(true); }

	std::string const &getLayout(void) const
	{ return _layout; }

	enum DirtyBits
	{
		DIRTY_VISIBLE = Distributed::DIRTY_CUSTOM << 0,
		DIRTY_LAYOUT = Distributed::DIRTY_CUSTOM << 1,
		DIRTY_CUSTOM = Distributed::DIRTY_CUSTOM << 2,
	};


/// Private virtual overrides
private :
	virtual void serialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits) const;

	virtual void deserialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits);

	virtual void _window_resetted(void) {}

private :
	Window(Window const &);
	Window &operator=(Window const &);

protected :
	bool _check_valid_window(void);

	CEGUI::Window *_window;
	vl::gui::GUI *_creator;

	std::string _layout;
	bool _visible;

};	// class Window

class ConsoleWindow : public Window
{
public :
	ConsoleWindow(vl::gui::GUI *creator);

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

// Private virtual overrides
private :
	virtual void _window_resetted(void);

private :
	std::deque<std::string> _console_memory;
	int _console_memory_index;
	std::string _console_last_command;

};	// class ConsoleWindow


}	// namespace gui

}	// namespace vl

#endif	// HYDRA_GUI_WINDOW_HPP
