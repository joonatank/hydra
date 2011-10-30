/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-10
 *	@file math/frustum.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

#include "frustum.hpp"

#include "base/exceptions.hpp"

/// ---------------------------------- Global --------------------------------
Ogre::Quaternion
vl::orientation_to_wall(vl::Wall const &wall)
{
	// Create the plane for transforming the head
	// Head doesn't need to be transformed for the view matrix
	// Using the plane to create a correct orientation for the view
	Ogre::Vector3 bottom_right( wall.bottom_right.at(0), wall.bottom_right.at(1), wall.bottom_right.at(2) );
	Ogre::Vector3 bottom_left( wall.bottom_left.at(0), wall.bottom_left.at(1), wall.bottom_left.at(2) );
	Ogre::Vector3 top_left( wall.top_left.at(0), wall.top_left.at(1), wall.top_left.at(2) );

	Ogre::Plane plane(bottom_right, bottom_left, top_left);

	// Transform the head
	// Should this be here? or should we only rotate the view matrix the correct angle
	Ogre::Vector3 cam_vec(-Ogre::Vector3::UNIT_Z);
	Ogre::Vector3 plane_normal = plane.normal.normalisedCopy();
	// Wall rotation used for orientating the frustum correctly
	// not used for any transformations
	return plane_normal.getRotationTo(cam_vec);
}

/// ---------------------------------- Frustum -------------------------------
vl::Frustum::Frustum(Type type)
	: _type(type)
	, _wall()
	, _near_clipping(0.01)
	, _far_clipping(100)
	, _head()
	, _head_frustum_x(false)
	, _head_frustum_y(true)
	, _head_frustum_z(false)
{
}

Ogre::Matrix4
vl::Frustum::getProjectionMatrix(void) const
{
	return getProjectionMatrix(0);
}

/// @todo this could be implemented using dirties to be lots of faster
/// then again how slow is it at the moment? is there a need for optimisation?
Ogre::Matrix4
vl::Frustum::getProjectionMatrix(vl::scalar eye_offset) const
{
	switch(_type)
	{
	case WALL:
		return _calculate_wall_projection(eye_offset);
	case FOV:
		return _calculate_fov_projection(eye_offset);
	default:
		// this should never happen
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Unknown projection type."));
	}
}


/// ------------------------------ Private -----------------------------------
Ogre::Matrix4
vl::Frustum::_calculate_wall_projection(vl::scalar eye_offset) const
{
	/* Projection matrix i.e. frustum
	 * | E	0	A	0 |
	 * | 0	F	B	0 |
	 * | 0	0	C	D |
	 * | 0	0	-1	0 |
	 *
	 * where
	 * A = -(right + left)/(right - left)
	 * B = -(top + bottom)/(top - bottom)
	 * C = -(far + near )/(far - near )
	 * D = -2*far*near/(far - near)
	 * E = 2*near/(right - left)
	 * F = 2*near/(top - bottom)
	 * some documents have B and C negative some positive, does not seem to make
	 * any difference at all.
	 */

	/// Necessary for calculating the frustum
	Ogre::Vector3 bottom_right(_wall.bottom_right[0], _wall.bottom_right[1], _wall.bottom_right[2]);
	Ogre::Vector3 bottom_left(_wall.bottom_left[0], _wall.bottom_left[1], _wall.bottom_left[2]);
	Ogre::Vector3 top_left(_wall.top_left[0], _wall.top_left[1], _wall.top_left[2]);

	// This is correct, it should not be inverse
	Ogre::Quaternion wallRot = orientation_to_wall(_wall);

	bottom_right = wallRot*bottom_right;
	bottom_left = wallRot*bottom_left;
	top_left = wallRot*top_left;

	// Calculate the frustum
	Ogre::Real wall_right = bottom_right.x;
	Ogre::Real wall_left = bottom_left.x;
	Ogre::Real wall_top = top_left.y;
	Ogre::Real wall_bottom = bottom_right.y;
	Ogre::Real wall_front = bottom_right.z;

	// @todo the head needs to be rotated by the wall so we get the
	// up axis in wall coordinates
	// this should fix the frustum for top and floor walls
	// necessary because for the top and floor walls the up axis
	// is z and not y as for the side walls

	// The coordinates right, left, top, bottom
	// represent a view frustum with coordinates (left, bottom, -near)
	// and (right, top, -near)
	//
	// So the wall and head needs to be scaled by the z-coordinate to
	// obtain the correct scale
	// If scale is negative it rotates 180 deg around z,
	// i.e. flips to the other side of the wall

	// Scale is necessary and is correct because 
	// if we increase it some of the object is clipped and not shown on either of the screens (too small fov)
	// and if we decrease it we the the same part on both front and side screens (too large fov) 
	Ogre::Real scale = -(wall_front)/_near_clipping;
	// Modify the front plane (or scale in this case)
	if(_head_frustum_z)
	{
		scale = -(wall_front - _head.position.z)/_near_clipping;
	}

	Ogre::Real right = wall_right/scale;
	Ogre::Real left = wall_left/scale;
	// Modify the right and left planes
	if(_head_frustum_x)
	{
		right = (wall_right - _head.position.x)/scale;
		left = (wall_left - _head.position.x)/scale;
	}

	// Golden ratio for the frustum
	// Should be tested with the head tracking if it doesn't work provide an
	// alternative to use golden ratio for one screen systems and non for VR
	Ogre::Real wall_height = wall_top - wall_bottom;
	Ogre::Real top = (wall_top - (1/PHI)*wall_height)/scale;
	Ogre::Real bottom = (wall_bottom - (1/PHI)*wall_height)/scale;

	// Modify the top and botoom planes
	if(_head_frustum_y)
	{
		top = (wall_top - _head.position.y)/scale;
		bottom = (wall_bottom - _head.position.y)/scale;
	}

	Ogre::Matrix4 projMat;

	// from Equalizer vmmlib/frustum
	projMat[0][0] = 2.0 * _near_clipping / ( right - left );
	projMat[0][1] = 0.0;
	projMat[0][2] = ( right + left ) / ( right - left );
	projMat[0][3] = 0.0;
    
	projMat[1][0] = 0.0;
	projMat[1][1] = 2.0 * _near_clipping / ( top - bottom );
	projMat[1][2] = ( top + bottom ) / ( top - bottom );
	projMat[1][3] = 0.0;

	projMat[2][0] = 0.0;
	projMat[2][1] = 0.0;
	// NOTE: Some glfrustum man pages say wrongly '(far + near) / (far - near)'
	projMat[2][2] = -(_far_clipping + _near_clipping) / (_far_clipping - _near_clipping);
	projMat[2][3] = -2.0 * _far_clipping * _near_clipping / (_far_clipping - _near_clipping);

	projMat[3][0] = 0.0;
	projMat[3][1] = 0.0;
	projMat[3][2] = -1.0;
	projMat[3][3] =  0.0;

	return projMat;
}


Ogre::Matrix4
vl::Frustum::_calculate_fov_projection(vl::scalar eye_offset) const
{
	BOOST_THROW_EXCEPTION(vl::not_implemented() << vl::desc("Fov projection not implemented."));
}
