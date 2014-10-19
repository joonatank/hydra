/**
 *	Copyright (c) 2011 - 2013 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-10
 *	@file math/frustum.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

#include "frustum.hpp"

#include "base/exceptions.hpp"

#include "base/string_utils.hpp"

/// ---------------------------------- Global --------------------------------
Ogre::Quaternion
vl::orientation_to_wall(vl::Wall const &wall)
{
	if(wall.empty())
	{ return Ogre::Quaternion::IDENTITY; }

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

std::ostream &
vl::operator<<(std::ostream &os, vl::Frustum const &f)
{
	// type, wall, clipping, fov, horizontal, asymmetric, fixed head, aspect,
	// user projection left and right
	// calculate the projection matrix for left and right with the parameters
	// we have and default ipd
	// @todo type should be converted to string
	os << "Frustum : " << "type = " << f.getType() << " wall = " << f.getWall()
		<< std::endl << " near clipping = " << f.getNearClipping()
		<< " far clipping = " << f.getFarClipping()
		<< " fov = " << f.getFov()
		<< " asymmetric frustum = " << vl::to_string(f.isAsymmetricStereoFrustum())
		<< " aspect = " << f.getAspect() << std::endl;

	return os;
}

/// ---------------------------------- Frustum -------------------------------
vl::Frustum::Frustum(Type type)
	: _type(type)
	, _wall()
	, _near_clipping(0.01)
	, _far_clipping(100)
	, _fov(Ogre::Degree(60))
	, _use_asymmetric_stereo(true)
	, _aspect(4.0/3)
{}

Ogre::Matrix4
vl::Frustum::getProjectionMatrix(void) const
{
	return getProjectionMatrix(Transform(), 0);
}

Ogre::Matrix4
vl::Frustum::getProjectionMatrix(vl::Transform const &head) const
{
	return getProjectionMatrix(head, 0);
}

/// @todo this could be implemented using dirties to be lots of faster
/// then again how slow is it at the moment? is there a need for optimisation?
Ogre::Matrix4
vl::Frustum::getProjectionMatrix(vl::Transform const &head, vl::scalar eye_offset) const
{
	switch(_type)
	{
	case WALL:
		return _calculate_wall_projection(head, eye_offset);
	case FOV:
		return _calculate_fov_projection(eye_offset);
	case USER:
		// old and not used keep it here till completely removed
	default:
		// this should never happen
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Unknown projection type."));
	}
}

/// ------------------------------ Private -----------------------------------
Ogre::Matrix4
vl::Frustum::_calculate_wall_projection(vl::Transform const &head_t, vl::scalar eye_offset) const
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
	if(_wall.empty())
	{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Wall can't be empty.")); }

	Plane plane(_wall);

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
	Ogre::Quaternion wallRot = orientation_to_wall(_wall);
	Ogre::Vector3 head = wallRot*head_t.position;

	Ogre::Vector3 eye = Ogre::Vector3::ZERO;
	if(_use_asymmetric_stereo)
	{
		// get the eye vector in head coordinates
		// then in the wall coordinates we are using for this wall
		eye = wallRot * head_t.quaternion * Ogre::Vector3(eye_offset, 0, 0);
	}

	// Scale is necessary and is correct because 
	// if we increase it some of the object is clipped and not shown on either of the screens (too small fov)
	// and if we decrease it we the the same part on both front and side screens (too large fov) 	
	Ogre::Real scale = -(plane.front - head.z - eye.z)/_near_clipping;

	Ogre::Real right = (plane.right - eye.x)/scale;
	Ogre::Real left = (plane.left - eye.x)/scale;

	// Modify the right and left planes
	right = (plane.right - head.x - eye.x)/scale;
	left = (plane.left - head.x - eye.x)/scale;

	// Modify the top and botom planes
	Ogre::Real top = (plane.top - head.y - eye.y)/scale;
	Ogre::Real bottom = (plane.bottom - head.y - eye.y)/scale;

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
	// Completely symmetric for now
	// @todo add eye_offset

	/* | E	0	A	0 |
	 * | 0	F	B	0 |
	 * | 0	0	C	D |
	 * | 0	0	-1	0 | */

	Ogre::Matrix4 projMat;
	/*
	float xymax = _near_clipping * std::tan(_fov.valueRadians());
	float ymin = -xymax;
	float xmin = -xymax;

	float width = xymax - xmin;
	float height = xymax - ymin;

	float depth = _far_clipping - _near_clipping;
	float C = -(_far_clipping + _near_clipping) / depth;
	float D = -2 * (_far_clipping * _near_clipping) / depth;

	float E = (2 * _near_clipping / width)/_aspect;
	float F = 2 * _near_clipping / height;
	*/

	/// @todo Doesn't work properly with Oculus Rift
	float near = _near_clipping;
	float far = _far_clipping;
	float depth = far - near;
	float f = 1/std::tan(_fov.valueRadians()/2);
	// for some reason we need to multiply by near clip distance even though
	// projection matrix documentation says we don't
	// if we don't we get 100 too close in z axis (so all movement is 1/100)
	// the only reliable resource for projection matrices is
	// http://www.songho.ca/opengl/gl_projectionmatrix.html
	// but it doesn't have a fov case

	float C = -far/depth;
	float D = -near*far/depth;
	float E = f/_aspect;
	float F = f;

	projMat[0][0] = E;
	projMat[0][1] = 0;
	projMat[0][2] = 0;
	projMat[0][3] = 0;

	projMat[1][0] = 0;
	projMat[1][1] = F;
	projMat[1][2] = 0;
	projMat[1][3] = 0;

	projMat[2][0] = 0;
	projMat[2][1] = 0;
	projMat[2][2] = C;
	projMat[2][3] = D;

	projMat[3][0] = 0;
	projMat[3][1] = 0;
	projMat[3][2] = -1;
	projMat[3][3] = 0;

	return projMat;
}
