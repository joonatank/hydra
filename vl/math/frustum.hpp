/**
 *	Copyright (c) 2011 - 2013 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-10
 *	@file math/frustum.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

#ifndef HYDRA_MATH_FRUSTUM_HPP
#define HYDRA_MATH_FRUSTUM_HPP

#include "base/wall.hpp"

#include "math/math.hpp"

#include "defines.hpp"

namespace vl
{

/// @brief calculate the orientation from eye space to wall
HYDRA_API Ogre::Quaternion orientation_to_wall(vl::Wall const &wall);

/// @todo only type supported for the moment is WALL
/// @todo there is a problem if either x or z is enabled and not the other one 
/// in three wall system, because x and z axes switch between front and side walls
///
/// @todo this is much more complicated than it should be
/// it does work for user defined projections, wall projections and fov projections
/// but it has way too many parameters and too many conditionals.
class HYDRA_API Frustum
{
public :
	enum Type
	{
		WALL,
		FOV,
		USER,	// user defined projection matrix
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

	Frustum(Type type = FOV);

	Ogre::Matrix4 getProjectionMatrix(void) const;

	Ogre::Matrix4 getProjectionMatrix(vl::Transform const &head) const;

	/// @brief calculate projetion matrix using VR parameters
	/// @param near_plane the near clipping plane distance
	/// @param far_plane the far clipping plane distance
	/// @param wall the projection wall/screen used for this frustum
	/// @param head the head transformation used for this projection
	/// @return OpenGL projection matrix
	/// @todo add asymmetric stereo frustum support (needs head rotation)
	/// @param eye_offset the offset of the current eye in stereo projection
	Ogre::Matrix4 getProjectionMatrix(vl::Transform const &head, vl::scalar eye_offset) const;

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

	/// Aspect ratio, only used for FOV based calculations
	vl::scalar getAspect(void) const
	{ return _aspect; }

	void setAspect(vl::scalar asp)
	{ _aspect = asp; }

	void enableAsymmetricStereoFrustum(bool enable)
	{ _use_asymmetric_stereo = enable; }

	bool isAsymmetricStereoFrustum(void) const
	{ return _use_asymmetric_stereo; }

	void setFov(Ogre::Radian const &fov)
	{ _fov = fov; }

	Ogre::Radian const &getFov(void) const
	{ return _fov; }

	void setUserProjection(Ogre::Matrix4 const &left, Ogre::Matrix4 const &right)
	{ _user_projection_left = left; _user_projection_right = right; }

	void setUserProjection(Ogre::Matrix4 const &p)
	{ _user_projection_left = p; _user_projection_right = p; }
	
	void setUserProjectionLeft(Ogre::Matrix4 const &p)
	{ _user_projection_left = p; }

	Ogre::Matrix4 const &getUserProjectionLeft(void) const
	{ return _user_projection_left; }

	void setUserProjectionRight(Ogre::Matrix4 const &p)
	{ _user_projection_right = p; }

	Ogre::Matrix4 const &getUserProjectionRight(void) const
	{ return _user_projection_right; }

	Ogre::Matrix4 _calculate_wall_projection(vl::Transform const &head, vl::scalar eye_offset) const;

	Ogre::Matrix4 _calculate_fov_projection(vl::scalar eye_offset) const;

private :

	Type _type;

	Wall _wall;

	vl::scalar _near_clipping;
	vl::scalar _far_clipping;

	Ogre::Radian _fov;

	bool _use_asymmetric_stereo;

	vl::scalar _aspect;

	Ogre::Matrix4 _user_projection_left;
	Ogre::Matrix4 _user_projection_right;

};	// class Frustum

std::ostream &operator<<(std::ostream &os, Frustum const &f);

}	// namespace vl

#endif	// HYDRA_MATH_FRUSTUM_HPP
