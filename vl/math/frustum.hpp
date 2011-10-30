/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-10
 *	@file math/frustum.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef HYDRA_MATH_FRUSTUM_HPP
#define HYDRA_MATH_FRUSTUM_HPP

#include "base/wall.hpp"

#include "math/math.hpp"

namespace vl
{

/// @brief calculate the orientation from eye space to wall
Ogre::Quaternion orientation_to_wall(vl::Wall const &wall);

/// @todo only type supported for the moment is WALL
class Frustum
{
public :
	enum Type
	{
		WALL,
		FOV,
	};

	Frustum(Type type = WALL);

	Ogre::Matrix4 getProjectionMatrix(void) const;

	/// @brief calculate projetion matrix using VR parameters
	/// @param near_plane the near clipping plane distance
	/// @param far_plane the far clipping plane distance
	/// @param wall the projection wall/screen used for this frustum
	/// @param head the head transformation used for this projection
	/// @return OpenGL projection matrix
	/// @todo add asymmetric stereo frustum support (needs head rotation)
	/// @param eye_offset the offset of the current eye in stereo projection
	Ogre::Matrix4 getProjectionMatrix(vl::scalar eye_offset) const;

	Type getType(void) const
	{ return _type; }
	
	void setType(Type type)
	{ _type = type; }

	Wall const &getWall(void) const
	{ return _wall; }

	void setWall(Wall const &wall)
	{ _wall = wall; }

	void setClipping(vl::scalar near_, vl::scalar far_)
	{
		_near_clipping = near_;
		_far_clipping = far_;
	}

	vl::scalar getNearClipping(void) const
	{ return _near_clipping; }

	vl::scalar getFarClipping(void) const
	{ return _far_clipping; }

	void setHeadTransformation(vl::Transform const &head)
	{ _head = head; }

	vl::Transform const &getHeadTransformation(void)
	{ return _head; }

	// The head tracker y needs to be present for there to be no disjoint
	// between side and front walls.
	void enableHeadFrustum(bool enable_x, bool enable_y, bool enable_z)
	{
		_head_frustum_x = enable_x;
		_head_frustum_y = enable_y;
		_head_frustum_z = enable_z;
	}

	void setFov(Ogre::Radian const &fov)
	{ _fov = fov; }

	Ogre::Radian const &getFov(void) const
	{ return _fov; }

private :

	Ogre::Matrix4 _calculate_wall_projection(vl::scalar eye_offset) const;

	Ogre::Matrix4 _calculate_fov_projection(vl::scalar eye_offset) const;

	Type _type;

	Wall _wall;

	vl::scalar _near_clipping;
	vl::scalar _far_clipping;

	vl::Transform _head;

	bool _head_frustum_x;
	bool _head_frustum_y;
	bool _head_frustum_z;

	Ogre::Radian _fov;

};	// class Frustum

}	// namespace vl

#endif	// HYDRA_MATH_FRUSTUM_HPP
