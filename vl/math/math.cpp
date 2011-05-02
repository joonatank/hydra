/**	@uathor Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-03
 *	@file math.cpp
 *
 */

#include "math.hpp"

/// ----------------------------- Transform ----------------------------------
bool 
vl::Transform::isIdentity(void) const
{
	return(position == Ogre::Vector3::ZERO && quaternion == Ogre::Quaternion::IDENTITY);
}

vl::Transform &
vl::Transform::operator*=(vl::Transform const &t)
{
	position += quaternion*t.position;
	quaternion = quaternion*t.quaternion;
	return *this;
}

vl::Transform &
vl::Transform::operator*=(Ogre::Matrix4 const &m)
{
	Ogre::Matrix4 m2( quaternion );
	m2.setTrans( position );
	Ogre::Matrix4 res = m2  * m;
	position = res.getTrans();
	quaternion = res.extractQuaternion();
	quaternion.normalise();
	return *this;
}

/// -------------------------------- Global ----------------------------------
std::ostream &
vl::operator<<( std::ostream &os, vl::Transform const &d )	
{
	Ogre::Radian rx, ry, rz;
	vl::getEulerAngles( d.quaternion, rx, ry, rz );
	os << "Position = (" << d.position.x << ", " << d.position.y << ", " << d.position.z << ")" 
		<< " : Orientation " << d.quaternion;
	/*
	os << " : Orientation in degrees yaw = " << d.quaternion.getYaw().valueDegrees() 
		<< " pitch = " << d.quaternion.getPitch().valueDegrees()
		<< " roll = " << d.quaternion.getRoll().valueDegrees();
	*/

	return os;
}

vl::Transform 
vl::operator*(vl::Transform const &t1, vl::Transform const &t2)
{
	vl::Transform temp(t1);
	temp *= t2;
	return temp;
}

vl::Transform 
vl::operator*( vl::Transform const &t, Ogre::Matrix4 const &m )
{
	vl::Transform temp(t);
	temp *= m;
	return temp;
}

vl::Transform 
vl::operator*( Ogre::Matrix4 const &m, vl::Transform const &t )
{
	Ogre::Matrix4 m2( t.quaternion );
	m2.setTrans( t.position );
	Ogre::Matrix4 res = m * m2;
	Ogre::Quaternion q = res.extractQuaternion();
	q.normalise();
	return vl::Transform( res.getTrans(), q );
}

bool
vl::operator==(vl::Transform const &t1, vl::Transform const &t2)
{
	return(t1.position == t2.position && t1.quaternion == t2.quaternion);
}

void 
vl::getEulerAngles( Ogre::Quaternion const &q, Ogre::Radian &x, Ogre::Radian &y, Ogre::Radian &z )
{
	vl::scalar test = q.x*q.y + q.z*q.w;
	if (test > 0.5-vl::epsilon)
	{ // singularity at north pole
		y = 2 * atan2(q.x,q.w);
		z = scalar(M_PI/2);
		x = 0;
		return;
	}
	if (test < -0.5+vl::epsilon)
	{ // singularity at south pole
		y = -2 * atan2(q.x,q.w);
		z = scalar(- M_PI/2);
		x = 0;
		return;
	}

	vl::scalar sqx = q.x*q.x;
	vl::scalar sqy = q.y*q.y;
	vl::scalar sqz = q.z*q.z;
	// heading
	y = ::atan2( scalar(2*q.y*q.w-2*q.x*q.z), scalar(1 - 2*sqy - 2*sqz) );
	// attitude
	z = ::asin( scalar(2*test) );
	// bank
	x = ::atan2( scalar(2*q.x*q.w-2*q.y*q.z ), scalar( 1 - 2*sqx - 2*sqz) );
}

void 
vl::fromEulerAngles( Ogre::Quaternion &q, Ogre::Radian const &rad_x,
					 Ogre::Radian const &rad_y, Ogre::Radian const &rad_z )
{
	vl::scalar heading = rad_y.valueRadians();
	vl::scalar attitude = rad_z.valueRadians();
	vl::scalar bank = rad_x.valueRadians();

	// Assuming the angles are in radians.
	vl::scalar c1 = ::cos(heading/2);
	vl::scalar s1 = ::sin(heading/2);
	vl::scalar c2 = ::cos(attitude/2);
	vl::scalar s2 = ::sin(attitude/2);
	vl::scalar c3 = ::cos(bank/2);
	vl::scalar s3 = ::sin(bank/2);
	vl::scalar c1c2 = c1*c2;
	vl::scalar s1s2 = s1*s2;
	q.w =c1c2*c3 - s1s2*s3;
	q.x =c1c2*s3 + s1s2*c3;
	q.y =s1*c2*c3 + c1*s2*s3;
	q.z =c1*s2*c3 - s1*c2*s3;
}

Ogre::Matrix4 
vl::calculate_projection_matrix(Ogre::Real c_near, Ogre::Real c_far,
								vl::EnvSettings::Wall const &wall)
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

	// Create the plane for transforming the head
	// TODO this is same code for both view and frustum combine them
	Ogre::Vector3 bottom_right( wall.bottom_right.at(0), wall.bottom_right.at(1), wall.bottom_right.at(2) );
	Ogre::Vector3 bottom_left( wall.bottom_left.at(0), wall.bottom_left.at(1), wall.bottom_left.at(2) );
	Ogre::Vector3 top_left( wall.top_left.at(0), wall.top_left.at(1), wall.top_left.at(2) );

	Ogre::Plane plane(bottom_right, bottom_left, top_left);

	// rotation from camera vector to wall
	Ogre::Vector3 cam_vec(-Ogre::Vector3::UNIT_Z);
	Ogre::Vector3 plane_normal = plane.normal.normalisedCopy();
	// This is correct, it should not be inverse
	Ogre::Quaternion wallRot = plane_normal.getRotationTo(cam_vec);

	bottom_right = wallRot*bottom_right;
	bottom_left = wallRot*bottom_left;
	top_left = wallRot*top_left;

	// Calculate the frustum
	Ogre::Real wall_right = bottom_right.x;
	Ogre::Real wall_left = bottom_left.x;
	Ogre::Real wall_top = top_left.y;
	Ogre::Real wall_bottom = bottom_right.y;
	Ogre::Real wall_front = bottom_right.z;

	// The coordinates right, left, top, bottom
	// represent a view frustum with coordinates (left, bottom, -near)
	// and (right, top, -near)
	//
	// So the wall and head needs to be scaled by the z-coordinate to
	// obtain the correct scale
	// If scale is negative it rotates 180 deg around z,
	// i.e. flips to the other side of the wall
	//
	// This comes because the front axis for every wall is different so for
	// front wall z-axis is the left walls x-axis.
	// Without the scale those axes will differ relative to each other.
	// Huh?
	// 

	// A decent constant which to transform the bottom and top
	// Basically pulled from the arm pit
	// T7 looks quite alright with this one, but your milage might vary.
	// Does still have huge problems with correct perspective front relative to sides.
	// And for example moving camera (modifying the view matrix) will create distortions
	// in the walls.
	Ogre::Real herwood_constant = 0.8;

	// Scale is necessary and is correct because 
	// if we increase it some of the object is clipped and not shown on either of the screens (too small fov)
	// and if we decrease it we the the same part on both front and side screens (too large fov) 
	Ogre::Real scale = -(wall_front)/c_near;

	Ogre::Real right = wall_right/scale;
	Ogre::Real left = wall_left/scale;

	// Golden ratio for the frustum
//	Ogre::Real phi = (1 + std::sqrt(5.0))/2;
	Ogre::Real wall_height = wall_top - wall_bottom;
//	Ogre::Real top = (wall_top - (1/phi)*wall_height)/scale;
//	Ogre::Real bottom = (wall_bottom - (1/phi)*wall_height)/scale;
	
	/// Projection plane needs to be centered around zero
	/// because the camera is in zero the top half will be above zero and the
	/// bottom half is below zero.
	Ogre::Real top = (wall_top - herwood_constant)/scale;
	Ogre::Real bottom = (wall_bottom - herwood_constant)/scale;

	Ogre::Real A = -(right + left)/(right - left);
	Ogre::Real B = -(top + bottom)/(top - bottom);
	Ogre::Real C = -(c_far + c_near)/(c_far - c_near);
	Ogre::Real D = -2*c_far*c_near/(c_far - c_near);
	Ogre::Real E = 2*c_near/(right - left);
	Ogre::Real F = 2*c_near/(top - bottom);

	Ogre::Matrix4 projMat;
	projMat[0][0] = E;
	projMat[0][1] = 0;
	projMat[0][2] = A;
	projMat[0][3] = 0;

	projMat[1][0] = 0;
	projMat[1][1] = F;
	projMat[1][2] = B;
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

Ogre::Quaternion
vl::orientation_to_wall(vl::EnvSettings::Wall const &wall)
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
