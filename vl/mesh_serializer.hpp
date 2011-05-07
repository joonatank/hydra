/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-02
 *	@file mesh_serializer.hpp
 *
 *	Mesh serializer used by exporters.
 *	Uses OgreMain library to do the mesh writing.
 *
 *	@todo add mesh deserializer for DotSceneLoader
 *	@todo remove the OgreMain as a dependency, we might need headers for defines
 *	but remove the main dependencies for creating the mesh.
 *	@todo skeleton serialization is completely missing
 *	@todo add mesh diff functions, one for telling if they are different,
 *	one for getting the difference and one for writing it, needs a custom
 *	format.
 */

#ifndef HYDRA_MESH_SERIALIZER_HPP
#define HYDRA_MESH_SERIALIZER_HPP

#include <OGRE/OgreMesh.h>
#include <OGRE/Ogre.h>
#include <OGRE/OgreDefaultHardwareBufferManager.h>

#include "mesh.hpp"

#include "typedefs.hpp"

namespace vl
{

class MeshWriter
{
public :
	MeshWriter( void );

	~MeshWriter( void );

	vl::MeshRefPtr createMesh(void);

	void writeMesh(vl::MeshRefPtr mesh, std::string const &filename);

private :

	Ogre::LogManager *_logMgr;
	Ogre::Math *_math;
	Ogre::LodStrategyManager *_lodMgr;
	Ogre::MaterialManager* _matMgr;
	Ogre::MeshSerializer* _meshSerializer;
	Ogre::DefaultHardwareBufferManager *_bufferMgr;
	Ogre::MeshManager* _meshMgr;
	Ogre::ResourceGroupManager* _resourcegm;
	Ogre::SkeletonManager *_skelMgr;
	Ogre::SkeletonSerializer *_skeletonSerializer;

};	// class MeshWriter

}	// namespace vl

#endif	// HYDRA_MESH_SERIALIZER_HPP
