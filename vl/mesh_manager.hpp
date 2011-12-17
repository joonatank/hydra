/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011/10 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-05
 *	@file mesh_manager.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef HYDRA_MESH_MANAGER_HPP
#define HYDRA_MESH_MANAGER_HPP

// Necessary for HYDRA_API
#include "defines.hpp"

#include "mesh.hpp"

#include "typedefs.hpp"

#include <OGRE/OgreVector3.h>
#include <stdint.h>

namespace vl
{

/// Callback for the one needing the mesh
struct MeshLoadedCallback
{
	virtual ~MeshLoadedCallback(void) {}

	virtual void meshLoaded(vl::MeshRefPtr mesh) = 0;
};

struct ManagerMeshLoadedCallback : public MeshLoadedCallback
{
	ManagerMeshLoadedCallback(std::string const &mesh_name, MeshManager *man)
		: name(mesh_name), owner(man)
	{}

	virtual void meshLoaded(vl::MeshRefPtr mesh);

	std::string name;
	MeshManager *owner;
};

/// Abstract interface for really loading a mesh
/// Can be either blocking of non-blocking, uses callbacks for both
/// For master the default implementation is blocking (because of the lack of threading system)
/// For slaves the only implementation is non-blocking
/// because blocking would screw up the Message system and the slaves are automatically
/// threaded with regards to resource loading (done by the master thread).
struct MeshLoaderCallback
{
	/// @param cb callback called when the Mesh is loaded (with the Mesh)
	virtual void loadMesh(std::string const &fileName, MeshLoadedCallback *cb) = 0;
};

/// blocking mesh loader for master
struct MasterMeshLoaderCallback : public MeshLoaderCallback
{
	MasterMeshLoaderCallback(vl::ResourceManagerRefPtr res_man);

	/// Blocks till the mesh is loaded and returns a valid mesh
	virtual void loadMesh(std::string const &fileName, MeshLoadedCallback *cb);

	vl::ResourceManagerRefPtr manager;
};

/// @todo add non-blocking callback for Master


class HYDRA_API MeshManager
{
public :
	MeshManager(MeshLoaderCallback *cb)
		: _load_callback(cb)
	{
		assert(_load_callback);
	}

	virtual ~MeshManager(void)
	{}

	/// @brief load a mesh
	/// @param file_name file name for the mesh file also used as mesh name
	/// If mesh with the name is already loaded returns it else loads a mesh 
	/// from file using the callback set for this manager.
	virtual vl::MeshRefPtr loadMesh(std::string const &file_name);

	/// @brief Non-blocking mesh loading, for masters this is still blocking
	/// Master will need a threaded loader which is not supplied for now
	virtual void loadMesh(std::string const &file_name, MeshLoadedCallback *cb);

	/// @todo not implemented
	virtual void writeMesh(vl::MeshRefPtr, std::string const &file_name);

	/// @brief creates a plane mesh
	/// @param name name for the mesh, used for storing it and writing it into a file
	/// @param normal the normal direction the plane is facing
	/// @param tessalation how many divisions there is in the mesh
	virtual vl::MeshRefPtr createPlane(std::string const &name,	Ogre::Real size_x = 1, Ogre::Real size_y = 1,
		 Ogre::Vector3 normal = Ogre::Vector3(0, 1, 0), uint16_t tesselation_x = 1, uint16_t tesselation_y = 1);

	/// @brief creates a sphere mesh
	/// @param name name for the mesh, used for storing it and writing it into a file
	/// @param radius the radius of the sphere in meters
	/// @param longitude how many divisions there is in the mesh vertically
	/// @param latitude how many divisions there is in the mesh horizontally
	virtual vl::MeshRefPtr createSphere(std::string const &name, Ogre::Real radius = 1, uint16_t longitude = 8, uint16_t latitude = 8);

	/// @brief creates a cube mesh with minimum polygon count
	/// @param name name for the mesh, used for storing it and writing it into a file
	/// @param size the size of the mesh in meters
	virtual vl::MeshRefPtr createCube(std::string const &name, Ogre::Vector3 size = Ogre::Vector3(1,1,1));
	/// @todo add version with tesselation
	/// @param tesselation how many divisions there is in the mesh
	//virtual vl::MeshPtr createCube(std::string const &name, Ogre::Vector3 size, uint16_t tesselation_x, uint16_t tesselation_y, uint16_t tesselation_z);

	/// @brief create some pre defined shapes available with default parameters
	/// @param type_name the name of the prefab type
	/// @return mesh for the prefab, already created if one exists or a new one
	/// Only single mesh instance is created for each of these types 
	/// so the tesselation and size parameters are equal.
	virtual vl::MeshRefPtr createPrefab(std::string const &type_name);

	/// @brief Get an already created mesh
	/// @param name the name of the Mesh
	/// @throw if no such mesh has been created
	vl::MeshRefPtr getMesh(std::string const &name);

	/// @brief is a mesh with the name already loaded
	bool hasMesh(std::string const &name) const;

	/// @todo not implemented
	/// @brief checks every Mesh loaded and unloads it if there is no users
	void cleanup_unused(void);

	/// @brief callback function
	void meshLoaded(std::string const &mesh_name, vl::MeshRefPtr mesh);

	typedef std::map<std::string, vl::MeshRefPtr> MeshMap;

private :
	MeshLoaderCallback *_load_callback;

	typedef std::map<std::string, std::vector<MeshLoadedCallback *> > ListenerMap;

	/// Meshes that are loaded in the background
	ListenerMap _waiting_for_loading;
	MeshMap _meshes;
};

}	// namespace vl

#endif	// HYDRA_MESH_MANAGER_HPP
