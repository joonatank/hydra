/**
 *	Copyright (c) 2011 Savant Simulators Oy
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-11
 *	@file cluster/object_types.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef HYDRA_CLUSTER_OBJECT_TYPES
#define HYDRA_CLUSTER_OBJECT_TYPES

namespace vl
{

enum OBJ_TYPE
{
	OBJ_INVALID,
	OBJ_PLAYER,
	OBJ_GUI,			// GUI object
	OBJ_GUI_CONSOLE,	// GUI Console window
	OBJ_SCENE_MANAGER,
	OBJ_SCENE_NODE,
	OBJ_ENTITY,
	OBJ_LIGHT,
	OBJ_CAMERA,
	OBJ_MOVABLE_TEXT,
	OBJ_RAY_OBJECT,
	OBJ_MATERIAL,		// Dynamic material definition
	OBJ_MATERIAL_MANAGER,
};

}	// namespace vl

#endif // HYDRA_CLUSTER_OBJECT_TYPES
