/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011/10 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-07
 *	@file: ray_cast.hpp
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
 *	Original version developed by gerds. Copied from OGRE wiki.
 */

#ifndef HYDRA_RAY_CAST_HPP
#define HYDRA_RAY_CAST_HPP

#include <OGRE/OgreEntity.h>
#include <OGRE/OgreVector3.h>
#include <OGRE/OgreQuaternion.h>
#include <OGRE/OgreSceneQuery.h>

#include "typedefs.hpp"

namespace vl
{
void GetMeshInformation(Ogre::Entity *entity, size_t &vertex_count, 
	Ogre::Vector3* &vertices, size_t &index_count, uint32_t* &indices,
	Ogre::Vector3 const &position, Ogre::Quaternion const &orient,
	Ogre::Vector3 const &scale);

// raycast from a point in to the scene.
// returns success or failure.
// on success the point is returned in the result.
class RayCast
{
public :
	RayCast(SceneManagerPtr scene);

	bool raycastFromPoint(Ogre::Vector3 const &point, Ogre::Vector3 const &normal, Ogre::Vector3 &result);

private:
	SceneManagerPtr _scene;
	Ogre::RaySceneQuery *_ray_scene_query;

};	// class RayCast

}	// namespace vl

#endif	// HYDRA_RAY_CAST_HPP
