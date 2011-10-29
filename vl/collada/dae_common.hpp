/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-10
 *	@file collada/dae_common.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef HYDRA_COLLADA_DAE_COMMON_HPP
#define HYDRA_COLLADA_DAE_COMMON_HPP

#include <COLLADAFramework/COLLADAFWMaterialBinding.h>

/// Necessary for Manager RefPtrs
#include "typedefs.hpp"
/// Necessary for Ogre::Radian cutoff angle
#include "math/types.hpp"
/// Necessary for SubMesh array typedef
#include "mesh.hpp"

namespace vl
{

namespace dae
{

typedef std::vector< std::pair<vl::SubMesh *, COLLADAFW::MaterialId> > SubMeshMaterialIDMap;

struct Managers
{
	vl::SceneManagerPtr scene;
	vl::MeshManagerRefPtr mesh;
	vl::MaterialManagerRefPtr material;

	// both kinematic and physics so that the exporter/importer can choose
	vl::KinematicWorldRefPtr kinematic;
	vl::physics::WorldRefPtr physics;
};

struct ImporterSettings
{
	// How to calculate normals for shading
	enum SHADING
	{
		SHADE_SMOOTH,		// calculate smooth normals everywhere
		SHADE_FLAT,			// every edge is sharp
		SHADE_COMBINED,		// use cut angle to smooth normals
	};

	// How to handle if some previous scene had same names for objects
	enum HANDLE_DUPLICATE_NAMES
	{
		HDN_RENAME,				// Rename the new object
		HDN_USE_ORIGINAL,		// Use the original object, discard new one
		HDN_UPDATE_ORIGINAL,	// Use the original, but Update it with the new information
		HDN_RAISE_ERROR,		// Any duplicated name will raise an error when imported
	};

	ImporterSettings(void)
		: shading(SHADE_SMOOTH)
		, normals_cutoff(Ogre::Radian(0))
		, attach_node(0)
		, base_material("bling_phong/shadows")
		, handle_duplicates(HDN_RENAME)
		, remove_duplicate_materials(false)
		, remove_duplicate_meshes(false)
	{}

	SHADING shading;

	Ogre::Radian normals_cutoff;

	// where to attach all scene nodes (root)
	vl::SceneNodePtr attach_node;

	// what material to use as a base for all materials
	std::string base_material;

	HANDLE_DUPLICATE_NAMES handle_duplicates;
	/// @brief checks all materials for exact duplicates and removes them
	/// exact duplicate here is all parameters except name
	bool remove_duplicate_materials;
	
	// traverses all the meshes removing exact duplicates
	// @todo not implemented
	bool remove_duplicate_meshes;

};	// struct ImporterSettings


}	// namespace dae

}	// namespace vl

#endif	// HYDRA_COLLADA_DAE_COMMON_HPP
