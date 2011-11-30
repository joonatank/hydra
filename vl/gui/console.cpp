/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-02
 *	@file gui/console.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

// Interface
#include "console.hpp"

// Necessary for passing messages back to creator
#include "gui.hpp"

/// CEGUI
#include <CEGUI/CEGUI.h>
#include <CEGUI/RendererModules/Ogre/CEGUIOgreRenderer.h>


#include <CEGUI/CEGUIDefaultResourceProvider.h>
#include <CEGUI/CEGUIImageset.h>
#include <CEGUI/CEGUIScheme.h>
#include <CEGUI/CEGUIInputEvent.h>

/// ---------------------------- ConsoleWindow ------------------------------
/// ------------------------------ Public -----------------------------------
vl::gui::ConsoleWindow::ConsoleWindow(vl::gui::GUI *creator)
	: Window(creator, "console.layout")
	, _console_memory_index(-1)	// Using -1 index to indicate not using memory
	, _py_error_colour(CEGUI::colour(0.5, 0.2, 0))
	, _py_out_colour(CEGUI::colour(0, 0.5, 0.5))
	, _error_colour(CEGUI::colour(0.5, 0, 0))
	, _out_colour(CEGUI::colour(0.2, 0.4, 0.8))
{
}

vl::gui::ConsoleWindow::~ConsoleWindow(void)
{
}


void
vl::gui::ConsoleWindow::printTo(std::string const &text, double time,
						 std::string const &type, vl::LOG_MESSAGE_LEVEL lvl)
{
	if(!_check_valid_window())
	{ return; }

	CEGUI::MultiColumnList *output = static_cast<CEGUI::MultiColumnList *>( _window->getChild("console/output") );
	assert( output );

	// Add time
	std::stringstream ss;
	ss << time;
	CEGUI::ListboxTextItem *item = new CEGUI::ListboxTextItem(ss.str());
	CEGUI::uint row = output->addRow(item, 1);

	// Set line number
	ss.str("");
	ss << row;
	item = new CEGUI::ListboxTextItem(ss.str());
	output->setItem(item, 0, row);

	// Add the text field
	CEGUI::String prefix;
	if( lvl == vl::LML_CRITICAL )
	{ prefix = "CRITICAL : "; }
	else if( lvl == vl::LML_TRIVIAL )
	{ prefix = "TRIVIAL : "; }

	item = new CEGUI::ListboxTextItem(prefix + CEGUI::String(text));
	// Save data type for filtering, HOW?
	if( type == "OUT" )
	{
		item->setTextColours(_out_colour);
	}
	else if( type == "ERROR" )
	{
		item->setTextColours(_error_colour);
	}
	else if( type == "PY_OUT" )
	{
		item->setTextColours(_py_out_colour);
	}
	else if( type == "PY_ERROR" )
	{
		item->setTextColours(_py_error_colour);
	}
	output->setItem(item, 2, row);
}

/// ------------------------- GECUI callbacks ----------------------------------
bool
vl::gui::ConsoleWindow::onConsoleInputAccepted( CEGUI::EventArgs const &e )
{
	if(!_check_valid_window())
	{ return true; }

	CEGUI::Editbox *input = static_cast<CEGUI::Editbox *>( _window->getChild("console/input") );
	assert( input );

	std::string command( input->getText().c_str() );

	if( command.size() > 0 )
	{
		input->setText("");

		if( *(command.end()-1) == ':' )
		{
			std::string str("Multi Line commands are not supported yet.");
			printTo(str, 0);
		}
		else
		{
			while( _console_memory.size() > 100 )
			{ _console_memory.pop_back(); }

			_console_memory.push_front(command);

			// TODO add support for time
			printTo(command, 0);

			_creator->sendCommand(command);
		}

		// Reset the memory index because the user has accepted the command
		_console_memory_index = -1;
		_console_last_command.clear();
	}

	return true;
}

bool
vl::gui::ConsoleWindow::onConsoleInputKeyDown(const CEGUI::EventArgs& e)
{
	if(!_check_valid_window())
	{ return true; }

	CEGUI::Editbox *input = static_cast<CEGUI::Editbox *>( _window->getChild("console/input") );
	assert( input );

	CEGUI::KeyEventArgs const &key = static_cast<CEGUI::KeyEventArgs const &>(e);
	if(key.scancode == CEGUI::Key::ArrowUp)
	{
		// Save the current user input when the list has not been scrolled
		if( _console_memory_index == -1 )
		{
			_console_last_command = input->getText().c_str();
		}

		++_console_memory_index;
		if( _console_memory_index >= _console_memory.size() )
		{ _console_memory_index = _console_memory.size()-1; }

		if( _console_memory_index > -1 )
		{
			std::string command = _console_memory.at(_console_memory_index);

			input->setText(command);
			input->setCaratIndex(input->getText().size());
		}

		return true;
	}
	else if(key.scancode == CEGUI::Key::ArrowDown)
	{
		--_console_memory_index;
		if( _console_memory_index < 0 )
		{
			_console_memory_index = -1;
			input->setText(_console_last_command);
		}
		else
		{
			std::string command = _console_memory.at(_console_memory_index);

			input->setText(command);
			input->setCaratIndex(input->getText().size());
		}

		return true;
	}

	return false;
}

bool
vl::gui::ConsoleWindow::onConsoleShow(const CEGUI::EventArgs& e)
{
	if(!_check_valid_window())
	{ return true; }

	CEGUI::Editbox *input = static_cast<CEGUI::Editbox *>( _window->getChild("console/input") );
	assert( input );

	input->activate();

	return true;
}

/// ------------------------------ Private -----------------------------------
void
vl::gui::ConsoleWindow::_window_resetted(void)
{
	assert(_window);
	// @todo needs a checking that the Window is correct

	_window->subscribeEvent(CEGUI::FrameWindow::EventShown, CEGUI::Event::Subscriber(&vl::gui::ConsoleWindow::onConsoleShow, this));

	CEGUI::MultiLineEditbox *output = static_cast<CEGUI::MultiLineEditbox *>( _window->getChild("console/output") );
	assert(output);
	assert(output->getVertScrollbar());
	output->getVertScrollbar()->setEndLockEnabled(true);

	CEGUI::Editbox *input = static_cast<CEGUI::Editbox *>( _window->getChild("console/input") );
	assert(input);
	input->subscribeEvent(CEGUI::Editbox::EventTextAccepted, CEGUI::Event::Subscriber(&vl::gui::ConsoleWindow::onConsoleInputAccepted, this));
	input->subscribeEvent(CEGUI::Editbox::EventKeyDown, CEGUI::Event::Subscriber(&vl::gui::ConsoleWindow::onConsoleInputKeyDown, this));
}
