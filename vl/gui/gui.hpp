/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-02
 *	@file gui/gui.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
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
#include <CEGUI/CEGUIWindow.h>

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
	GUI(vl::Session *session, uint64_t id, vl::CommandCallback *cb);
	
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

	void initGUI(vl::Window *win);
	void initGUIResources(vl::Settings const &set);
	void addGUIResourceGroup(std::string const &name, fs::path const &path);

	EditorWindowRefPtr getEditor(void)
	{ return _editor; }

	ConsoleWindowRefPtr getConsole(void)
	{ return _console; }

	bool isVisible(void) const;

	void sendCommand(std::string const &cmd);

	CEGUI::Window *getRoot(void) const
	{ return _root; }

	enum DirtyBits
	{
		DIRTY_CUSTOM = Distributed::DIRTY_CUSTOM << 0,
	};

/// Private virtual overrides
private :
	virtual void serialize( cluster::ByteStream &msg, const uint64_t dirtyBits ) const;

	virtual void deserialize( cluster::ByteStream &msg, const uint64_t dirtyBits );

private :
	GUI(GUI const &);
	GUI &operator=(GUI const &);

/// Data
private :

	vl::gui::EditorWindowRefPtr _editor;
	vl::gui::ConsoleWindowRefPtr _console;

	std::vector<vl::gui::WindowRefPtr> _windows;
	std::vector<vl::gui::WindowRefPtr> _editors_without_parent;

	vl::CommandCallback *_cmd_cb;

	vl::Session *_session;

	CEGUI::Window *_root;

};	// class GUI

}	// namespace gui

}	// namespace vl

#endif	// HYDRA_GUI_GUI_HPP
