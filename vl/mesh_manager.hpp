/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-05
 *	@file mesh_manager.hpp
 *
 */

#ifndef HYDRA_MESH_MANAGER_HPP
#define HYDRA_MESH_MANAGER_HPP

#include "mesh.hpp"

#include "typedefs.hpp"

#include <OGRE/OgreVector3.h>
#include <stdint.h>

namespace vl
{

/// Abstract interface for really loading a mesh
/// Implementations:
/// Master blocking and non-blocking
/// Slave blocking and non-blocking
struct MeshLoaderCallback
{
	/// For blocking loader returns the Mesh created
	/// For non-blocking loader returns NULL
	virtual vl::MeshRefPtr loadMesh(std::string const &fileName) = 0;
};

struct BlockingMeshLoaderCallback : public MeshLoaderCallback
{

};

/// blocking mesh loader for master
struct MasterMeshLoaderCallback : public BlockingMeshLoaderCallback
{
	MasterMeshLoaderCallback(vl::ResourceManagerRefPtr res_man);

	/// Blocks till the mesh is loaded and returns a valid mesh
	virtual vl::MeshRefPtr loadMesh(std::string const &fileName);

	vl::ResourceManagerRefPtr manager;
};

/// @todo add non-blocking callbacks

/// @todo add a callback for loading the actual mesh
/// for Master this callback loads a file using ResourceManager
/// for slaves this callback sends a load message to Server
/// both blocking and non-blocking versions can be provided
class MeshManager
{
public :
	MeshManager(MeshLoaderCallback *cb)
		: _load_callback(cb)
	{
		assert(_load_callback);
	}

	virtual ~MeshManager(void)
	{}

	/// @todo only precreated mesh loading is implemented
	virtual vl::MeshRefPtr loadMesh(std::string const &file_name, bool block = true);

	/// @todo not implemented
	virtual void writeMesh(vl::MeshRefPtr, std::string const &file_name);

	/// @brief creates a plane mesh
	/// @param name name for the mesh, used for storing it and writing it into a file
	/// @param normal the normal direction the plane is facing
	/// @param tessalation how many divisions there is in the mesh
	virtual vl::MeshRefPtr createPlane(std::string const &name,	Ogre::Real size_x, Ogre::Real size_y,
		 Ogre::Vector3 normal = Ogre::Vector3(0, 1, 0), uint16_t tesselation_x = 1, uint16_t tesselation_y = 1);

	/// @todo not implemented
	/// @brief creates a sphere mesh
	/// @param name name for the mesh, used for storing it and writing it into a file
	/// @param radius the radius of the sphere in meters
	/// @param longitude how many divisions there is in the mesh vertically
	/// @param latitude how many divisions there is in the mesh horizontally
	virtual vl::MeshRefPtr createSphere(std::string const &name, Ogre::Real radius, uint16_t longitude, uint16_t latitude);

	/// @todo not implemented
	/// @brief creates a cube mesh with minimum polygon count
	/// @param name name for the mesh, used for storing it and writing it into a file
	/// @param size the size of the mesh in meters
	virtual vl::MeshRefPtr createCube(std::string const &name, Ogre::Vector3 size);
	/// @todo add version with tesselation
	/// @param tesselation how many divisions there is in the mesh
	//virtual vl::MeshPtr createCube(std::string const &name, Ogre::Vector3 size, uint16_t tesselation_x, uint16_t tesselation_y, uint16_t tesselation_z);

	/// @brief Get an already created mesh
	/// @param name the name of the Mesh
	/// @throw if no such mesh has been created
	vl::MeshRefPtr getMesh(std::string const &name);

	/// @brief is a mesh with the name already loaded
	bool hasMesh(std::string const &name) const;

	/// @todo not implemented
	/// @brief checks every Mesh loaded and unloads it if there is no users
	virtual void cleanup_unused(void);

	typedef std::map<std::string, vl::MeshRefPtr> MeshMap;

private :
	MeshLoaderCallback *_load_callback;

	MeshMap _meshes;
};

}	// namespace vl

#endif	// HYDRA_MESH_MANAGER_HPP
