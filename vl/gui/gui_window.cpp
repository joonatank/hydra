/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-05
 *	@file GUI/gui_window.cpp
 *
 *	This file is part of Hydra VR game engine.
 */

/// Interface
#include "gui_window.hpp"

/// GUI
#include <CEGUI/CEGUI.h>
#include <CEGUI/RendererModules/Ogre/CEGUIOgreRenderer.h>
#include <CEGUI/CEGUISystem.h>
#include <CEGUI/CEGUIWindow.h>
#include <CEGUI/CEGUIWindowManager.h>
#include <CEGUI/CEGUIDefaultResourceProvider.h>
#include <CEGUI/CEGUIImageset.h>
#include <CEGUI/CEGUIScheme.h>
#include <CEGUI/CEGUIInputEvent.h>

#include <OGRE/OgreLogManager.h>

// Necessary for passing messages back to creator
#include "gui.hpp"

/// --------------------------------- Window ---------------------------------
/// --------------------------------- Public ---------------------------------
vl::gui::Window::Window(vl::gui::GUI *creator, std::string const &layout)
	: _window(0)
	, _creator(creator)
	, _layout(layout)
{
	assert(_creator);
}

void
vl::gui::Window::setVisible(bool visible)
{
	if(_visible != visible)
	{
		setDirty(DIRTY_VISIBLE);
		_visible = visible;
	}
}


/// --------------------------------- Private --------------------------------
void
vl::gui::Window::serialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits) const
{
	if(dirtyBits & DIRTY_LAYOUT)
	{
		msg << _layout;
	}

	if(dirtyBits & DIRTY_VISIBLE)
	{
		msg << _visible;
	}
}

void
vl::gui::Window::deserialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits)
{
	bool reset = false;

	if(dirtyBits & DIRTY_LAYOUT)
	{
		msg >> _layout;
		/// Layout resetting is not supported
		if(!_window)
		{
			reset = true;
		}
	}

	if(dirtyBits & DIRTY_VISIBLE)
	{
		msg >> _visible;
		if(_window)
		{ _window->setVisible(_visible); }
	}

	if(reset)
	{
		/// @todo should create an empty CEGUI window
		if(_layout.empty())
		{ BOOST_THROW_EXCEPTION(vl::not_implemented()); }

		std::clog << "Creating window with a layout : " << _layout << std::endl;
		_window = CEGUI::WindowManager::getSingleton().loadWindowLayout(_layout);
		assert(_creator->getRoot());
		_creator->getRoot()->addChildWindow(_window);
		// Copy parameters
		_window->setVisible(_visible);

		// Inform derived classes
		_window_resetted();
	}
}

bool
vl::gui::Window::_check_valid_window(void)
{
	if(!_window)
	{
		std::cout << "gui::Window : No native Window set so functinality does not work." << std::endl;
		return false;
	}
	else
	{
		return true;
	}
}

/// ---------------------------- ConsoleWindow ------------------------------
/// ------------------------------ Public -----------------------------------
vl::gui::ConsoleWindow::ConsoleWindow(vl::gui::GUI *creator)
	: Window(creator, "console.layout")
	, _console_memory_index(-1)	// Using -1 index to indicate not using memory
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
		item->setTextColours(CEGUI::colour(0, 0.2, 0.4));
	}
	else if( type == "ERROR" )
	{
		item->setTextColours(CEGUI::colour(0.5, 0, 0));
	}
	else if( type == "PY_OUT" )
	{
		item->setTextColours(CEGUI::colour(0, 0.5, 0.5));
	}
	else if( type == "PY_ERROR" )
	{
		item->setTextColours(CEGUI::colour(0.5, 0.2, 0));
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

/// ---------------------------- EditorWindow -------------------------------
/// ------------------------------ Public -----------------------------------
vl::gui::EditorWindow::EditorWindow(vl::gui::GUI *creator)
	: Window(creator, "editor.layout")
{
}

vl::gui::EditorWindow::~EditorWindow(void)
{
}

/// ------------------------ Public CEGUI callbacks ----------------------------
bool
vl::gui::EditorWindow::onNewClicked( CEGUI::EventArgs const &e )
{
	std::string msg("vl::Window::onNewClicked");
	Ogre::LogManager::getSingleton().logMessage(msg, Ogre::LML_TRIVIAL);
	return true;
}

bool
vl::gui::EditorWindow::onOpenClicked( CEGUI::EventArgs const &e )
{
	std::string msg("vl::Window::onOpenClicked");
	Ogre::LogManager::getSingleton().logMessage(msg, Ogre::LML_TRIVIAL);
	return true;
}

bool
vl::gui::EditorWindow::onSaveClicked( CEGUI::EventArgs const &e )
{
	std::string msg("vl::Window::onSaveClicked");
	Ogre::LogManager::getSingleton().logMessage(msg, Ogre::LML_TRIVIAL);
	return true;
}

bool
vl::gui::EditorWindow::onQuitClicked( CEGUI::EventArgs const &e )
{
	std::string msg("vl::Window::onQuitClicked");
	Ogre::LogManager::getSingleton().logMessage(msg, Ogre::LML_TRIVIAL);
	_creator->sendCommand( "quit()" );

	return true;
}

bool
vl::gui::EditorWindow::onResetClicked( CEGUI::EventArgs const &e )
{
	std::string msg("vl::Window::onResetClicked");
	Ogre::LogManager::getSingleton().logMessage(msg, Ogre::LML_TRIVIAL);
	return true;
}

bool
vl::gui::EditorWindow::onImportSceneClicked( CEGUI::EventArgs const &e )
{
	std::string msg("vl::Window::onImportSceneClicked");
	Ogre::LogManager::getSingleton().logMessage(msg, Ogre::LML_TRIVIAL);
	return true;
}

bool
vl::gui::EditorWindow::onReloadScenesClicked( CEGUI::EventArgs const &e )
{
	std::string msg("vl::Window::onReloadScenesClicked");
	Ogre::LogManager::getSingleton().logMessage(msg, Ogre::LML_TRIVIAL);
	return true;
}


bool
vl::gui::EditorWindow::onNewScriptClicked( CEGUI::EventArgs const &e )
{
	std::string msg("vl::Window::onNewScriptClicked");
	Ogre::LogManager::getSingleton().logMessage(msg, Ogre::LML_TRIVIAL);
	return true;
}

bool
vl::gui::EditorWindow::onAddScriptClicked( CEGUI::EventArgs const &e )
{
	std::string msg("vl::Window::onAddScriptClicked");
	Ogre::LogManager::getSingleton().logMessage(msg, Ogre::LML_TRIVIAL);
	return true;
}

bool
vl::gui::EditorWindow::onReloadScriptsClicked( CEGUI::EventArgs const &e )
{
	std::string msg("vl::Window::onReloadScriptsClicked");
	Ogre::LogManager::getSingleton().logMessage(msg, Ogre::LML_TRIVIAL);
	return true;
}

bool
vl::gui::EditorWindow::onShowAxisChanged( CEGUI::EventArgs const &e )
{
	std::string msg("vl::Window::onShowAxisChanged");
	Ogre::LogManager::getSingleton().logMessage(msg, Ogre::LML_TRIVIAL);
	return true;
}

bool
vl::gui::EditorWindow::onShowNamesChanged( CEGUI::EventArgs const &e )
{
	std::string msg("vl::Window::onShowNamesChanged");
	Ogre::LogManager::getSingleton().logMessage(msg, Ogre::LML_TRIVIAL);
	return true;
}

bool
vl::gui::EditorWindow::onShowJointsChanged( CEGUI::EventArgs const &e )
{
	std::string msg("vl::Window::onShowJointsChanged");
	Ogre::LogManager::getSingleton().logMessage(msg, Ogre::LML_TRIVIAL);
	return true;
}

/// ------------------------------ Private -----------------------------------
void
vl::gui::EditorWindow::_window_resetted(void)
{
	assert(_window);
	// @todo should check that we have a correct type of a window

	CEGUI::MenuItem *item = static_cast<CEGUI::MenuItem *>( _window->getChildRecursive("editor/newItem") );
	assert( item );
	item->subscribeEvent(CEGUI::MenuItem::EventClicked, CEGUI::Event::Subscriber(&vl::gui::EditorWindow::onNewClicked, this));

	item = static_cast<CEGUI::MenuItem *>( _window->getChildRecursive("editor/openItem") );
	assert( item );
	item->subscribeEvent(CEGUI::MenuItem::EventClicked, CEGUI::Event::Subscriber(&vl::gui::EditorWindow::onOpenClicked, this));

	item = static_cast<CEGUI::MenuItem *>( _window->getChildRecursive("editor/saveItem") );
	assert( item );
	item->subscribeEvent(CEGUI::MenuItem::EventClicked, CEGUI::Event::Subscriber(&vl::gui::EditorWindow::onSaveClicked, this));

	item = static_cast<CEGUI::MenuItem *>( _window->getChildRecursive("editor/quitItem") );
	assert( item );
	item->subscribeEvent(CEGUI::MenuItem::EventClicked, CEGUI::Event::Subscriber(&vl::gui::EditorWindow::onQuitClicked, this));

	item = static_cast<CEGUI::MenuItem *>( _window->getChildRecursive("editor/resetItem") );
	assert( item );
	item->subscribeEvent(CEGUI::MenuItem::EventClicked, CEGUI::Event::Subscriber(&vl::gui::EditorWindow::onResetClicked, this));

	item = static_cast<CEGUI::MenuItem *>( _window->getChildRecursive("editor/importSceneItem") );
	assert( item );
	item->subscribeEvent(CEGUI::MenuItem::EventClicked, CEGUI::Event::Subscriber(&vl::gui::EditorWindow::onImportSceneClicked, this));

	item = static_cast<CEGUI::MenuItem *>( _window->getChildRecursive("editor/reloadScenes") );
	assert( item );
	item->subscribeEvent(CEGUI::MenuItem::EventClicked, CEGUI::Event::Subscriber(&vl::gui::EditorWindow::onReloadScenesClicked, this));

	item = static_cast<CEGUI::MenuItem *>( _window->getChildRecursive("editor/addScriptItem") );
	assert( item );
	item->subscribeEvent(CEGUI::MenuItem::EventClicked, CEGUI::Event::Subscriber(&vl::gui::EditorWindow::onAddScriptClicked, this));

	item = static_cast<CEGUI::MenuItem *>( _window->getChildRecursive("editor/newScriptItem") );
	assert( item );
	item->subscribeEvent(CEGUI::MenuItem::EventClicked, CEGUI::Event::Subscriber(&vl::gui::EditorWindow::onNewScriptClicked, this));

	item = static_cast<CEGUI::MenuItem *>( _window->getChildRecursive("editor/reloadScripts") );
	assert( item );
	item->subscribeEvent(CEGUI::MenuItem::EventClicked, CEGUI::Event::Subscriber(&vl::gui::EditorWindow::onReloadScriptsClicked, this));

	CEGUI::Checkbox *checkBox = static_cast<CEGUI::Checkbox *>( _window->getChildRecursive("editor/showAxes") );
	assert( checkBox );
	checkBox->subscribeEvent(CEGUI::Checkbox::EventCheckStateChanged, CEGUI::Event::Subscriber(&vl::gui::EditorWindow::onShowAxisChanged, this));

	checkBox = static_cast<CEGUI::Checkbox *>( _window->getChildRecursive("editor/showNames") );
	assert( checkBox );
	checkBox->subscribeEvent(CEGUI::Checkbox::EventCheckStateChanged, CEGUI::Event::Subscriber(&vl::gui::EditorWindow::onShowNamesChanged, this));

	checkBox = static_cast<CEGUI::Checkbox *>( _window->getChildRecursive("editor/showJoints") );
	assert( checkBox );
	checkBox->subscribeEvent(CEGUI::Checkbox::EventCheckStateChanged, CEGUI::Event::Subscriber(&vl::gui::EditorWindow::onShowJointsChanged, this));
}
