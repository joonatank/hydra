/**
 *	Copyright (c) 2010 - 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-12
 *	@file game_manager.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

#ifndef HYDRA_FLAGS_HPP
#define HYDRA_FLAGS_HPP

namespace vl
{

/**	@enum LOADER_FLAGS
 *	@brief change the loader behavior
 *	Even though these are flags not all of them are compatible with each other
 *
 *	Flags:
 *	Rename, renames duplicated objects
 *	Overwrite, overwrites any duplicated objects this is mostly useful for reloading scenes
 *
 *	@todo is there a better way to handle options that are incompatible with each other?
 */
enum LOADER_FLAGS
{
	LOADER_FLAG_NONE = 0,
	LOADER_FLAG_RENAME = 1,
	LOADER_FLAG_OVERWRITE = 1 << 1,
};

}	// namespace vl

#endif // HYDRA_FLAGS_HPP
