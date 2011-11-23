/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-07
 *	@file input/input.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

/*
 *	Basic input interface.
 */

#ifndef HYDRA_INPUT_INPUT_HPP
#define HYDRA_INPUT_INPUT_HPP

namespace vl
{

class InputDevice
{
public :
	virtual ~InputDevice(void) {}

};	// class InputDevice

}	// namespace vl

#endif	// HYDRA_INPUT_INPUT_HPP
