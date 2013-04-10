/**
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-05
 *	@file gui/gui_window.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

#ifndef HYDRA_GUI_WINDOW_HPP
#define HYDRA_GUI_WINDOW_HPP

#include <string>
#include <deque>

// Base class
#include "cluster/distributed.hpp"
// Necessary for distribution
#include "cluster/session.hpp"

// Necessary for log level
#include "logger.hpp"

#include "typedefs.hpp"

// Concrete implementation
#include "Gorilla.h"

#include <OIS/OISKeyboard.h>

#include <boost/signal.hpp>

#include "defines.hpp"

namespace vl
{

namespace gui
{
/** @class Window
 *	@brief Wrapper around CEGUI Window, provides the callbacks
 */
class HYDRA_API Window : public vl::Distributed
{
	typedef boost::signal<void ()> NativeCreated;

public :

	Window(vl::gui::GUI *creator, std::string const &layout = std::string());

	virtual ~Window(void);

	bool isVisible(void) const
	{ return _visible; }

	void setVisible(bool visible);

	void toggleVisible(void)
	{ setVisible(!isVisible()); }

	void hide(void)
	{ setVisible(false); }

	void show(void)
	{ setVisible(true); }

	std::string const &getLayout(void) const
	{ return _layout; }

	enum DirtyBits
	{
		DIRTY_VISIBLE = Distributed::DIRTY_CUSTOM << 0,
		DIRTY_LAYOUT = Distributed::DIRTY_CUSTOM << 1,
		DIRTY_CUSTOM = Distributed::DIRTY_CUSTOM << 2,
	};

	virtual void injectKeyDown(OIS::KeyEvent const &key) {}
	virtual void injectKeyUp(OIS::KeyEvent const &key) {}

	boost::signals::connection addListener(NativeCreated::slot_type const &slot)
	{ return _signal.connect(slot); }

	void removeListener(boost::signals::connection subscriber)
	{ subscriber.disconnect(); }

	/// @internal called from GUI every frame
	/// Should decide if this Window needs a redraw and proceed to draw if necessary.
	/// @todo these various update methods need to be renamed properly
	void update(void);

/// Private virtual overrides
private :
	void serialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits) const;

	void deserialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits);

	/// Template method pattern, we override the masters version and childs are
	/// required to override these if they need to add to the functionality
	virtual void doSerialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits) const {}

	virtual void doDeserialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits) {}

	virtual void _window_resetted(void) {}
	
	/// @todo these various update methods need to be renamed properly
	virtual void _update(void) {}

private :

	Window(Window const &);
	Window &operator=(Window const &);

protected :
	bool _check_valid_window(void);

	vl::gui::GUI *_creator;

	Gorilla::Screen *mScreen;

	std::string _layout;
	bool _visible;

	bool _reset;

	NativeCreated _signal;

};	// class Window

}	// namespace gui

}	// namespace vl

#endif	// HYDRA_GUI_WINDOW_HPP
