/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-02
 *	@file gui/console.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef HYDRA_GUI_CONSOLE_HPP
#define HYDRA_GUI_CONSOLE_HPP

#include "gui_window.hpp"

namespace vl
{

namespace gui
{

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

	CEGUI::colour _py_error_colour;
	CEGUI::colour _py_out_colour;
	CEGUI::colour _error_colour;
	CEGUI::colour _out_colour;

};	// class ConsoleWindow

}	// namespace gui

}	// namespace vl

#endif	// HYDRA_GUI_CONSOLE_HPP
