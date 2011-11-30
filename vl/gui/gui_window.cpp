/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-05
 *	@file gui/gui_window.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

/// Interface
#include "gui_window.hpp"

// Necessary for attaching windows
#include "gui.hpp"

// Necessary for creating the real windows
#include <CEGUI/CEGUISystem.h>
#include <CEGUI/CEGUIWindowManager.h>

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

	doSerialize(msg, dirtyBits);
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

		_signal();
	}

	doDeserialize(msg, dirtyBits);
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
