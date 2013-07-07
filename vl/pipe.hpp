/**
 *	Copyright (c) 2011 - 2013 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2013-07
 *	@file pipe.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

#ifndef HYDRA_PIPE_HPP
#define HYDRA_PIPE_HPP

// Base class
#include "cluster/distributed.hpp"
// Parent
#include "cluster/session.hpp"
// Necessary for creating windows
#include "window_interface.hpp"
// Necessary for config::Window struct
#include "base/envsettings.hpp"

namespace vl
{
/**	@class Pipe
 *	@brief Distributed representation of a single GPU 
 *	At this moment a single cluster Node.
 *
 *	Handles creation of Windows so we can dynamically add new Windows
 *	Master holds N Pipes where N is the number of Renderers
 *	Slaves hold 1 Pipe i.e. every Renderer has a single Pipe
 */
class Pipe : public Distributed
{
public :
	/// Master constructor
	Pipe(vl::Session *session, std::string const &name);
	
	/// Slave constructor
	/// @todo I think we need to pass Renderer here on slaves right?
	Pipe(vl::Session *session, uint64_t id);

	~Pipe(void);

	std::string const &getName(void) const
	{ return _name; }

	/// Rendering functions
	/// Do not call directly
	void capture(void);
	
	void draw(void);
	
	void swap(void);

	/// parameters
	bool isDebugOverlay(void) const
	{ return _debug_overlay_enabled; }
	void enableDebugOverlay(bool enable);

	bool isGUI(void) const
	{ return _gui_enabled; }
	void enableGUI(bool enable);

	/// @brief
	/// @param winConf definition for the window
	vl::IWindow *createWindow(vl::config::Window const &winConf);

	/// Slave function
	/// Window has been created
	void _windowCreated(Window *win);

	/// Dirty functions to get Renderer working without overhauling
	std::vector<vl::Window *> &getWindows(void)
	{ return _windows; }

	std::vector<vl::Window *> const &getWindows(void) const
	{ return _windows; }

	/// @todo this should be a common function for all distributed objects
	Session *getSession(void)
	{ return _session; }

	enum DirtyBits
	{
		DIRTY_NAME = vl::Distributed::DIRTY_CUSTOM << 0,
		DIRTY_WINDOWS = vl::Distributed::DIRTY_CUSTOM << 1,
		DIRTY_PARAMS = vl::Distributed::DIRTY_CUSTOM << 2,
		DIRTY_CUSTOM = vl::Distributed::DIRTY_CUSTOM << 3,
	};

private :
	virtual void serialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits) const;

	virtual void deserialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits);

	bool _hasWindow(uint64_t id);
	Window *_findWindow(uint64_t id);

	Session *_session;

	std::string _name;

	std::vector<vl::Window *> _windows;
	// Window ids stored when we don't yet have the window
	std::vector<uint64_t> _window_ids;
	
	/// Configuration
	bool _gui_enabled;
	bool _debug_overlay_enabled;

};	// class Pipe

}	// namespace vl

#endif // HYDRA_PIPE_HPP
