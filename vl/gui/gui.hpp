/**
 *	Copyright (c) 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-02
 *	@file gui/gui.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 */


#ifndef HYDRA_GUI_GUI_HPP
#define HYDRA_GUI_GUI_HPP

// Base class
#include "cluster/distributed.hpp"

#include "cluster/session.hpp"

// Necessary for command callback
#include "renderer_interface.hpp"

// Concrete implementation
#include "Gorilla.h"

#include <OIS/OISKeyboard.h>
#include <OIS/OISMouse.h>

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
	GUI(vl::Session *session, uint64_t id);
	
	~GUI(void);

	/// @brief Master creator
	WindowRefPtr createWindow(std::string const &type, std::string const &name, std::string const &layout);

	WindowRefPtr createWindow(std::string const &type)
	{ return createWindow(type, std::string(), std::string()); }

	WindowRefPtr createWindow(std::string const &type, std::string const &name)
	{ return createWindow(type, name, std::string()); }

	/// @brief Slave creator
	WindowRefPtr createWindow(vl::OBJ_TYPE, uint64_t id);

	WindowRefPtr getWindow(uint64_t id);

	WindowRefPtr getWindow(std::string const &name);

	void initGUI(Ogre::Viewport *view);

	ConsoleWindowRefPtr getConsole(void)
	{ return _console; }

	bool isVisible(void) const;

	void sendCommand(std::string const &cmd);

	Gorilla::Screen *createScreen(void);

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

/// Data
private :

	vl::gui::ConsoleWindowRefPtr _console;

	std::vector<vl::gui::WindowRefPtr> _windows;

	vl::Session *_session;

	Gorilla::Silverback *_gorilla;
	Ogre::Viewport *mViewport;

	//CEGUI::Window *_root;

};	// class GUI

}	// namespace gui

}	// namespace vl

#endif	// HYDRA_GUI_GUI_HPP
