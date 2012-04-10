/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011/10 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-03
 *	@file renderer_interface.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef HYDRA_RENDERER_INTERFACE_HPP
#define HYDRA_RENDERER_INTERFACE_HPP

#include "base/envsettings.hpp"
#include "typedefs.hpp"

#include "cluster/message.hpp"

#include "logger.hpp"

#include <string>

#include "window_interface.hpp"

#include <boost/signal.hpp>

namespace vl
{

/**	@class RendererInterface
 *	@brief Abstract interface for the renderer
 */
class RendererInterface : public LogReceiver
{
protected :
	typedef boost::signal<void (std::string const &)> CommandSent;
	typedef boost::signal<void (vl::cluster::EventData const &)> EventSent;

public :

	virtual ~RendererInterface(void) {}

	virtual void setMeshManager(vl::MeshManagerRefPtr mesh_man) = 0;

	/// @brief interface through which the Renderer is controlled
// 	virtual void handleMessage(vl::cluster::Message &msg) = 0;

	/// @brief specialisation of handleMessage
	virtual void setProject(vl::Settings const &settings) = 0;
	
	virtual void updateScene(vl::cluster::Message &msg) = 0;
	
	virtual void createSceneObjects(vl::cluster::Message &msg) = 0;

	/// @brief push event upwards to Application
	virtual void sendEvent(vl::cluster::EventData const &event) = 0;

	virtual void sendCommand(std::string const &cmd) = 0;

	virtual bool guiShown(void) const = 0;

	// Needed by slave and master for controlling the application run
	virtual void init(vl::config::EnvSettingsRefPtr env) = 0;

	/// Loop functions

	/// @brief Capture input from all the Windows
	virtual void capture(void) = 0;

	/// @brief Will render all the windows
	virtual void draw(void) = 0;
	
	/// @brief Will swap all the windows
	virtual void swap(void) = 0;

	// TODO do we need all these getters
	// These are needed in Window
	virtual vl::ogre::RootRefPtr getRoot( void ) = 0;

	virtual vl::config::EnvSettingsRefPtr getEnvironment( void ) = 0;

	virtual vl::Player const &getPlayer( void ) const = 0;

	virtual vl::Player &getPlayer(void) = 0;
	
	virtual vl::Player *getPlayerPtr(void) = 0;
	
	virtual vl::MeshManagerRefPtr getMeshManager(void) = 0;

	virtual vl::config::Window const &getWindowConf(std::string const &name) const = 0;

	virtual vl::IWindow *createWindow(vl::config::Window const &winConf) = 0;

	virtual void addCommandListener(CommandSent::slot_type const &slot) = 0;

	virtual void addEventListener(EventSent::slot_type const &slot) = 0;

	virtual void enableDebugOverlay(bool enable) = 0;

	virtual bool isDebugOverlayEnabled(void) const = 0;

	virtual gui::GUIRefPtr getGui(void) = 0;

};	// class RendererInterface

}	// namespace vl

#endif	// HYDRA_RENDERER_INTERFACE_HPP
