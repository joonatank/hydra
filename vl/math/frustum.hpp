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
/// @todo there is a problem if either x or z is enabled and not the other one 
/// in three wall system, because x and z axes switch between front and side walls
class Frustum
{
public :
	enum Type
	{
		WALL,
		FOV,
	};

	struct Plane
	{
		Plane(void)
			: right(0)
			, left(0)
			, top(0)
			, bottom(0)
			, front(0)
		{}

		Plane(vl::Wall const &wall)
		{
			Ogre::Vector3 bottom_right(wall.bottom_right[0], wall.bottom_right[1], wall.bottom_right[2]);
			Ogre::Vector3 bottom_left(wall.bottom_left[0], wall.bottom_left[1], wall.bottom_left[2]);
			Ogre::Vector3 top_left(wall.top_left[0], wall.top_left[1], wall.top_left[2]);

			// This is correct, it should not be inverse
			Ogre::Quaternion wallRot = orientation_to_wall(wall);

			bottom_right = wallRot*bottom_right;
			bottom_left = wallRot*bottom_left;
			top_left = wallRot*top_left;

			right = bottom_right.x;
			left = bottom_left.x;
			top = top_left.y;
			bottom = bottom_right.y;
			front = bottom_right.z;
		}

		Ogre::Real right;
		Ogre::Real left;
		Ogre::Real top;
		Ogre::Real bottom;
		Ogre::Real front;
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

	void enableTransformationModifications(bool enable)
	{ _transformation_modifications = enable; }

	bool isTransformationModifications(void) const
	{ return _transformation_modifications; }

	Transform getModificationTransformation(void) const;

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

	bool _transformation_modifications;

};	// class Frustum

}	// namespace vl

#endif	// HYDRA_MATH_FRUSTUM_HPP
