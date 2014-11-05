/**
 *	Copyright (c) 2012 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-02
 *	@file gui/console.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

#ifndef HYDRA_GUI_CONSOLE_HPP
#define HYDRA_GUI_CONSOLE_HPP

#include "gui_window.hpp"

namespace vl
{

namespace gui
{

class HYDRA_API ConsoleWindow : public Window
{
	typedef boost::signal<void (std::string const &)> CommandSent;

public :
	ConsoleWindow(vl::gui::GUI *creator);

	~ConsoleWindow(void);

	void shutdown();
    
	void printTo(std::string const &text, double time,
						std::string const &type = std::string(),
						vl::LOG_MESSAGE_LEVEL lvl = vl::LML_NORMAL);

	virtual void injectKeyDown(OIS::KeyEvent const &key);
	virtual void injectKeyUp(OIS::KeyEvent const &key);

	bool wantsLogging(void) const
	{ return true; }

	virtual void addCommandListener(CommandSent::slot_type const &slot)
	{ _command_signal.connect(slot); }

// Private virtual overrides
private :
	virtual void _window_resetted(void);

	virtual void _update(void);

private :
	void _updateConsole(void);

	void _updatePrompt(void);

	void _print(Ogre::String const &text);

	void _input_accepted(void);

private :
	// Console memory
	std::deque<std::string> _console_memory;
	int _console_memory_index;
	std::string _console_last_command;

	Ogre::ColourValue _py_error_colour;
	Ogre::ColourValue _py_out_colour;
	Ogre::ColourValue _error_colour;
	Ogre::ColourValue _out_colour;

	CommandSent _command_signal;

	// For gorilla
	Gorilla::Layer *mLayer;
	Gorilla::Caption*    mPromptText;
	Gorilla::MarkupText* mConsoleText;
	Gorilla::Rectangle*  mDecoration;
	Gorilla::GlyphData*  mGlyphData;

	bool mUpdateConsole;
	bool mUpdatePrompt;
	bool mIsInitialised;

	// Console data
	Ogre::String _prompt;
	int mStartline;
	std::list<Ogre::String> _lines;

	uint16_t _caret_position;

};	// class ConsoleWindow

}	// namespace gui

}	// namespace vl

#endif	// HYDRA_GUI_CONSOLE_HPP
