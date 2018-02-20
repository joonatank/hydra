/**
 *	Copyright (c) 2012 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-02
 *	@file gui/gui.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */


#ifndef HYDRA_GUI_GUI_HPP
#define HYDRA_GUI_GUI_HPP

// Base class
#include "cluster/distributed.hpp"

#include "cluster/session.hpp"

// Necessary for Object types (for now hard coded)
#include "cluster/object_types.hpp"

#include "typedefs.hpp"

// Concrete implementation
#include "Gorilla.h"
// Necessary for OIS::KeyEvent
#include <OIS/OISKeyboard.h>
#include <OIS/OISMouse.h>

namespace vl
{

namespace gui
{

enum GUI_SCALE
{
	GS_NORMAL,
	GS_HMD,
};

/**	@class GUI
 *	@brief Distributed class for GUI modifications
 *
 *	@todo figure out where this is updated/rendered and document
 *	add general process documentation here
 *	add specific method documentation
 *
 *	@todo add global scale variable to scale all the windows
 *	primarily for Oculus since the windows/fonts need to be scaled for it.
 *	We need to 
 *		- find font size we want to use
		- replace CONSOLE_FONT_INDEX with dynamic version
		- make configurations for two or three different font sizes
		- we need to use fractions to modify the scale off all windows
			or maybe not, since only few of them use borders
			some like the console is full screen we just need to add more lines
 *
 */
class HYDRA_API GUI : public vl::Distributed
{
public :
	/// @brief Master constructor
	GUI(vl::Session *session);

	/// @brief Slave or Renderer constructor
	/// @param session the distributed session this GUI is attached to
	/// @param id ID of this GUI, has to be valid
	GUI(vl::Session *session, uint64_t id);
	
	~GUI(void);

	/// @brief Master creation
	gui::WindowRefPtr createWindow(vl::OBJ_TYPE type);

	/// @brief Slave creator
	gui::WindowRefPtr createWindow(vl::OBJ_TYPE type, uint64_t id);

	gui::WindowRefPtr getWindow(uint64_t id);

	gui::WindowRefPtr getWindow(std::string const &name);

	vl::Channel *getChannel(void)
	{ return _channel; }

	GUI_SCALE getScale(void) const
	{ return _scale; }

	/// @brief setScale
	/// Needs to be called before initialised
	void setScale(GUI_SCALE scale);

	void initialise(vl::Channel *view);

	ConsoleWindowRefPtr getConsole(void)
	{ return _console; }
	
	PerformanceOverlayRefPtr getPerformanceOverlay(void)
	{ return _overlay; }

	bool isVisible(void) const;

	/// Called from Windows to create the actual draw object
	Gorilla::Screen *createScreen(void);
	
	void createMouseCursor(Gorilla::Screen *screen);
	void showMouseCursor(void);
	void hideMouseCursor(void);

	bool initialised(void) const;

	void injectKeyDown(OIS::KeyEvent const &key);
	void injectKeyUp(OIS::KeyEvent const &key);

	void injectMouseEvent(OIS::MouseEvent const &evt);

	enum DirtyBits
	{
		DIRTY_CUSTOM = Distributed::DIRTY_CUSTOM << 0,
	};

	void update(void);

/// Private virtual overrides
private :
	virtual void serialize( cluster::ByteStream &msg, const uint64_t dirtyBits ) const;

	virtual void deserialize( cluster::ByteStream &msg, const uint64_t dirtyBits );

private :
	GUI(GUI const &);
	GUI &operator=(GUI const &);

	/// Initialisation function used by all constructors
	void _clear(void);
	
	/// Initialise the GUI
	/// @internal, called when deserialised
	void _init(void);

/// Data
private :

	vl::gui::ConsoleWindowRefPtr _console;
	vl::gui::PerformanceOverlayRefPtr _overlay;

	std::vector<vl::gui::WindowRefPtr> _windows;

	GUI_SCALE _scale;

	vl::Session *_session;

	vl::Channel *_channel;

	Gorilla::Silverback *_gorilla;

	Gorilla::Layer *_mouse_cursor_layer;
	Gorilla::Rectangle *_mouse_cursor;

};	// class GUI

}	// namespace gui

}	// namespace vl

#endif	// HYDRA_GUI_GUI_HPP
