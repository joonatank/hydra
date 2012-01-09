/**
 *	Copyright (c) 2011 Tampere University of Technology
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-02
 *	@file mesh_serializer.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

/**
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

// Base class
#include "serializer.hpp"

#include "resource_manager.hpp"
#include "mesh_serializer_impl.hpp"

namespace vl
{

class MeshSerializer : public Serializer
{
public :
	MeshSerializer(void);

	~MeshSerializer(void);

	vl::MeshRefPtr createMesh(void);

	void writeMesh(vl::MeshRefPtr mesh, std::string const &filename);

	void readMesh(vl::MeshRefPtr mesh, vl::Resource &res);

private :
	/// Wrapper around Ogre Managers, created if Ogre is not initialised
	struct OgreManagers {
		OgreManagers(void);

		~OgreManagers(void);
		
		Ogre::LogManager *logMgr;
		Ogre::Math *math;
		Ogre::LodStrategyManager *lodMgr;
		Ogre::MaterialManager *matMgr;
		Ogre::DefaultHardwareBufferManager *bufferMgr;
		Ogre::MeshManager* meshMgr;
		Ogre::ResourceGroupManager* resGroupMgr;
		Ogre::SkeletonManager *skelMgr;
	};

	OgreManagers *_ogre_mgr;

};	// class MeshWriter

}	// namespace vl

#endif	// HYDRA_MESH_SERIALIZER_HPP
