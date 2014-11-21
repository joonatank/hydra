/**
 *	Copyright (c) 2012 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-02
 *	@file gui/gui.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

// Interface
#include "gui.hpp"

// gui window, necessary for the windows this creates
#include "gui_window.hpp"
#include "console.hpp"
#include "performance_overlay.hpp"

// Necessary for init GUI
#include "window.hpp"
// Necessary for creating the Canvas
#include "channel.hpp"

#include "base/exceptions.hpp"

vl::gui::GUI::GUI(vl::Session *session)
{
	_clear();
	_session = session;
	assert(_session);
	_session->registerObject(this, OBJ_GUI);
}

vl::gui::GUI::GUI(vl::Session *session, uint64_t id)
{
	_clear();
	_session = session;
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
vl::gui::GUI::createWindow(vl::OBJ_TYPE t)
{
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
	case OBJ_GUI_PERFORMANCE_OVERLAY:
		_overlay.reset(new PerformanceOverlay(this));
		win = _overlay;
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
	if(_viewport && _viewport->getCamera())
	{	
		return _gorilla->createScreen(_viewport, "dejavu"); 
	}
	
	return 0;
}

void
vl::gui::GUI::createMouseCursor(Gorilla::Screen *screen)
{
	// for some reason this function is called multiple times
	// using if instead of assert
	// only create one mouse cursor
	if(!_mouse_cursor_layer && !_mouse_cursor)
	{
		_mouse_cursor_layer = screen->createLayer(15);
		_mouse_cursor = _mouse_cursor_layer->createRectangle(0, 0, 10, 18);
		_mouse_cursor->background_image("mousepointer");
		_mouse_cursor_layer->show();
	}
}

void
vl::gui::GUI::hideMouseCursor(void)
{
	if(_mouse_cursor_layer)
	{ _mouse_cursor_layer->hide(); }
	
}

void
vl::gui::GUI::showMouseCursor(void)
{
	if(_mouse_cursor_layer)
	{ _mouse_cursor_layer->show();}
}


bool
vl::gui::GUI::initialised(void) const
{
	return(_viewport && _viewport->getCamera());
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
		_console->injectKeyUp(key);
	}
}

void
vl::gui::GUI::injectMouseEvent(OIS::MouseEvent const &evt)
{
	// there is a possibility that the mouse cursor has not yet
	// been created while the GUI has been created.
	if(_mouse_cursor)
	{
		_mouse_cursor->position(evt.state.X.abs,evt.state.Y.abs);
	}
}

void
vl::gui::GUI::setChannel(vl::Channel *view)
{
	if(_channel)
	{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Resetting Channel is not supported.")); }

	if(!view)
	{ BOOST_THROW_EXCEPTION(vl::null_pointer() << vl::desc("Called without Channel.")); }

	if(!view->getRenderViewport())
	{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("No Ogre::Viewport")); }

	_channel = view;
	_viewport = _channel->getRenderViewport();
}

bool
vl::gui::GUI::isVisible(void) const
{
	/// @todo should iterate over all windows, and check all that have
	/// value wantsInput in them
	/// Actually we need two visibility settings in Windows
	/// to determine if it counts as GUI visible or not.
	/// For example performance overlay (or anyother overlay) should not be
	/// counted as a GUI visibility.
	return( initialised() && _console && _console->isVisible() );
}

void
vl::gui::GUI::update(void)
{
	for(std::vector<vl::gui::WindowRefPtr>::iterator iter = _windows.begin();
		iter != _windows.end(); ++iter)
	{
		(*iter)->update();
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
	if( DIRTY_ALL & dirtyBits )
	{
		// Should be fine here when GUI is created
		_init();
	}
}

void
vl::gui::GUI::_clear(void)
{
	_session = 0;
	_channel = 0;
	_gorilla = 0;
	_viewport = 0;
	_mouse_cursor_layer = 0;
	_mouse_cursor = 0;
}

void
vl::gui::GUI::_init()
{
	std::clog << "vl::gui::GUI::_init" << std::endl;

	// Can not create Gorilla in Constructor because we don't have the resources yet
	assert(!_gorilla);
	_gorilla = new Gorilla::Silverback();
	_gorilla->loadAtlas("dejavu");
}
