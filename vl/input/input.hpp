/**	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-07
 *	@file input/input.hpp
 *
 *	This file is part of Hydra a VR game engine.
 *
 *	Basic input interface.
 */

#ifndef HYDRA_INPUT_INPUT_HPP
#define HYDRA_INPUT_INPUT_HPP

namespace vl
{

class InputDevice
{
public :
	virtual void mainloop(void) = 0;
};

}

#endif