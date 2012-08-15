/**
 *	Copyright (c) 2011 - 2012 Savant Simulators Oy
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-11
 *	@file cluster/object_types.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

#ifndef HYDRA_CLUSTER_OBJECT_TYPES
#define HYDRA_CLUSTER_OBJECT_TYPES

namespace vl
{

// @todo we should divide this into primary objects
// gui objects and movable objects
// @todo should also not be hard coded
// And should be completely unique
enum OBJ_TYPE
{
	OBJ_INVALID,
	OBJ_PLAYER,
	OBJ_GUI,			// Master GUI object
	OBJ_GUI_CONSOLE,	// GUI Console window
	OBJ_GUI_PERFORMANCE_OVERLAY,	// GUI performance overlay
	OBJ_SCENE_MANAGER,
	OBJ_SCENE_NODE,
	OBJ_MATERIAL,		// Dynamic material definition
	OBJ_MATERIAL_MANAGER,
	// Movable Objects
	// Don't put anything below these we are using greater than compares
	OBJ_MOVABLE,
	OBJ_ENTITY,
	OBJ_LIGHT,
	OBJ_CAMERA,
	OBJ_MOVABLE_TEXT,
	OBJ_RAY_OBJECT,
};

}	// namespace vl

#endif // HYDRA_CLUSTER_OBJECT_TYPES
