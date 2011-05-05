/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-05
 *	@file transform.cpp
 *
 */

#include "transform.hpp"

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
//	Ogre::Radian rx, ry, rz;
//	vl::getEulerAngles( d.quaternion, rx, ry, rz );
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
