/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-03
 *	@file renderer_interface.hpp
 *
 */

#ifndef VL_RENDERER_INTERFACE_HPP
#define VL_RENDERER_INTERFACE_HPP

#include "base/envsettings.hpp"
#include "typedefs.hpp"

#include "cluster/message.hpp"

#include "logger.hpp"

#include <string>

namespace vl
{

/// callback functors
struct MsgCallback : public Callback
{
	virtual ~MsgCallback(void) {}

	virtual void operator()(vl::cluster::Message const &) = 0;
};

struct CommandCallback : public Callback
{
	virtual ~CommandCallback(void) {}

	virtual void operator()(std::string const &) = 0;
};

/**	@class RendererInterface
 *	@brief Abstract interface for the renderer
 */
class RendererInterface : public LogReceiver
{
public :

	virtual ~RendererInterface(void) {}

	virtual void setMeshManager(vl::MeshManagerRefPtr mesh_man) = 0;

	/// @brief interface through which the Renderer is controlled
// 	virtual void handleMessage(vl::cluster::Message &msg) = 0;

	/// @brief specialisation of handleMessage
	virtual void setProject(vl::cluster::Message &msg) = 0;
	
	virtual void initScene(vl::cluster::Message &msg) = 0;
	
	virtual void updateScene(vl::cluster::Message &msg) = 0;
	
	virtual void createSceneNodes(vl::cluster::Message &msg) = 0;
	
	virtual void print(vl::cluster::Message &msg) = 0;

	// Interface for sending local changes
// 	void sendMessage(vl::cluster::Message const &msg)
// 	{
// 		vl::cluster::Message msg_cpy(msg);
// 		handleMessage(msg_cpy);
// 	}

	virtual void sendEvent(vl::cluster::EventData const &event) = 0;

	virtual void sendCommand(std::string const &cmd) = 0;

	virtual bool guiShown(void) const = 0;

	// Needed by slave and master for controlling the application run
	virtual void init(vl::EnvSettingsRefPtr env) = 0;

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

	virtual vl::EnvSettingsRefPtr getEnvironment( void ) = 0;

	virtual vl::Player const &getPlayer( void ) const = 0;

	virtual vl::MeshManagerRefPtr getMeshManager(void) = 0;

	virtual vl::EnvSettings::Window getWindowConf(std::string const &name) = 0;

	// Callbacks
//	virtual void setRegisterForOutputCallback(RendererCallback *cb) = 0;

//	virtual void setInputEventCallback(RendererMsgCallback *cb) = 0;

	virtual void setSendMessageCB(MsgCallback *cb) = 0;

};	// class RendererInterface

struct RendererCommandCallback : public CommandCallback
{
	RendererCommandCallback(RendererInterface *rend)
		: renderer(rend)
	{}

	virtual void operator()(std::string const &cmd)
	{
		assert(renderer);
		renderer->sendCommand(cmd);
	}

	RendererInterface *renderer;
};

}	// namespace vl

#endif	// VL_RENDERER_INTERFACE_HPP
