/**
 *	Copyright (c) 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-02
 *	@file gui/gui.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 */

// Interface
#include "gui.hpp"

// gui window, necessary for the windows this creates
#include "gui_window.hpp"
#include "console.hpp"

#include "settings.hpp"
// Necessary for init GUI
#include "window.hpp"

vl::gui::GUI::GUI(vl::Session *session)
	: _session(session)
	, _gorilla(0)
	, mViewport(0)
{
	assert(_session);
	_session->registerObject(this, OBJ_GUI);
}

vl::gui::GUI::GUI(vl::Session *session, uint64_t id)
	: _session(session)
	, _gorilla(0)
	, mViewport(0)
{
	assert(_session);

	if(id == vl::ID_UNDEFINED)
	{ BOOST_THROW_EXCEPTION(vl::invalid_id()); }

	_session->registerObject(this, OBJ_GUI, id);
}

vl::gui::GUI::~GUI(void)
{
	// @todo destroy Gorilla
	// Something is really wrong about the destructors as this also crashes
	// when exiting the program.
	//delete _gorilla;
}

vl::gui::WindowRefPtr
vl::gui::GUI::createWindow(std::string const &type, std::string const &name, std::string const &layout)
{
	/// @todo add name support

	OBJ_TYPE t;
	if(type == "console")
	{
		t = OBJ_GUI_CONSOLE;
	}
	else
	{
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Unknown GUI window type"));
	}

	return createWindow(t, 0);
}

vl::gui::WindowRefPtr
vl::gui::GUI::createWindow(vl::OBJ_TYPE t, uint64_t id)
{
	vl::gui::WindowRefPtr win;
	switch(t)
	{
	case OBJ_GUI_CONSOLE:
		_console.reset(new ConsoleWindow(this));
		win = _console;
		break;
	default :
		std::cout << vl::CRITICAL << "GUI::createWindow : Incorrect type for Window" << std::endl;
		BOOST_THROW_EXCEPTION(vl::exception());
	}
	
	assert(win);
	_session->registerObject(win.get(), t, id);

	_windows.push_back(win);

	return win;
}

vl::gui::WindowRefPtr
vl::gui::GUI::getWindow(uint64_t id)
{
	for(std::vector<vl::gui::WindowRefPtr>::iterator iter = _windows.begin();
		iter != _windows.end(); ++iter)
	{
		if((*iter)->getID() == id)
		{ return *iter; }
	}

	return WindowRefPtr();
}

vl::gui::WindowRefPtr
vl::gui::GUI::getWindow(std::string const &name)
{
	BOOST_THROW_EXCEPTION(vl::not_implemented());
	
	return WindowRefPtr();
}

Gorilla::Screen *
vl::gui::GUI::createScreen(void)
{
	if(mViewport && mViewport->getCamera())
	{ return _gorilla->createScreen(mViewport, "dejavu"); }
	
	return 0;
}

bool
vl::gui::GUI::initialised(void) const
{
	return(mViewport && mViewport->getCamera());
}

void
vl::gui::GUI::injectKeyDown(OIS::KeyEvent const &key)
{
	// Only console supported for now
	if(_console)
	{
		_console->injectKeyDown(key);
	}
}

void
vl::gui::GUI::injectKeyUp(OIS::KeyEvent const &key)
{
	// Only console supported for now
	if(_console)
	{
		//_console->onKeyPressed(key);
	}
}

void
vl::gui::GUI::injectMouseEvent(OIS::MouseEvent const &evt)
{
	// Nothing uses these for now
}


void
vl::gui::GUI::initGUI(Ogre::Viewport *view)
{
	std::string message("vl::gui::GUI::_initGUI");
	std::clog << message << std::endl;

	// @todo Gorilla::Silverback should be in the GUI
	assert(!_gorilla);
	_gorilla = new Gorilla::Silverback();
	_gorilla->loadAtlas("dejavu");
	mViewport = view;
}

bool
vl::gui::GUI::isVisible(void) const
{
	/// @todo should iterate over all windows, and check all that have
	/// value wantsInput in them
	return( initialised() && _console && _console->isVisible() );
}

void 
vl::gui::GUI::sendCommand(std::string const &cmd)
{
	// @todo missing a signal
}

void
vl::gui::GUI::update(void)
{
	if(_console)
	{
		_console->update();
	}
}

/// --------------------------------- Private --------------------------------
void
vl::gui::GUI::serialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits ) const
{
}

void 
vl::gui::GUI::deserialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits )
{
}
