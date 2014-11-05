/**
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-12
 *	@file window_interface.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */


#ifndef HYDRA_WINDOW_INTERFACE_HPP
#define HYDRA_WINDOW_INTERFACE_HPP

#include "typedefs.hpp"

#include <stdint.h>

namespace vl
{

class IWindow
{
public :

	/// @brief set the same camera to all Channels
	virtual void setCamera(vl::CameraPtr camera) = 0;

	virtual std::string const &getName( void ) const = 0;

	virtual void takeScreenshot(std::string const &prefix, std::string const &suffix) = 0;

	/// @brief return the system handle for the window
	virtual uint64_t getHandle(void) const = 0;

	/// @brief Get wether hardware stereo is enabled or not
	/// @return true if the window has stereo enabled
	virtual bool hasStereo(void) const = 0;

	/// Capture input events
	virtual void capture(void) = 0;

	/// Instruct the Channels to draw the Scene
	virtual void draw(void) = 0;

	/// Swap the back buffer to front
	virtual void swap(void) = 0 ;

	virtual void resize(int w, int h) = 0;

};	// IWindow

}	// namespace vl

#endif // HYDRA_WINDOW_INTERFACE_HPP
