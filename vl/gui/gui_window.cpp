/**
 *	Copyright (c) 2012 Savant Simulators
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

/// Interface
#include "gui_window.hpp"

// Necessary for attaching windows
#include "gui.hpp"

/// --------------------------------- Window ---------------------------------
/// --------------------------------- Public ---------------------------------
vl::gui::Window::Window(vl::gui::GUI *creator, std::string const &layout)
	: _creator(creator)
	, _layout(layout)
	, mScreen(0)
	, _reset(false)
{
	assert(_creator);
}

vl::gui::Window::~Window(void)
{
	// @todo destroy screen
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


void
vl::gui::Window::update(void)
{
	if(_reset && _creator->initialised())
	{
		std::clog << "Creating Gorilla window" << std::endl;
		assert(!mScreen);
		// Create Gorilla window
		mScreen = _creator->createScreen();
		assert(mScreen);
		
		// @warning: Create mouse cursor (this is so wrong!)
		_creator->createMouseCursor(mScreen);

		// Copy parameters
		mScreen->setVisible(_visible);

		_reset = false;

		// Inform derived classes
		_window_resetted();

		_signal();
	}

	_update();
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
	if(dirtyBits & DIRTY_LAYOUT)
	{
		msg >> _layout;

		if(!mScreen)
		{
			std::clog << "Should reset GUI window." << std::endl;
			_reset = true; 
		}
	}

	if(dirtyBits & DIRTY_VISIBLE)
	{
		msg >> _visible;
		if(mScreen)
		{ mScreen->setVisible(_visible); }
	}

	doDeserialize(msg, dirtyBits);
}

bool
vl::gui::Window::_check_valid_window(void)
{
	if(!mScreen)
	{
		std::cout << "gui::Window : No native Window set so functinality does not work." << std::endl;
		return false;
	}
	else
	{
		return true;
	}
}
