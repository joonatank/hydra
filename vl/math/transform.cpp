/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-05
 *	@file math/transform.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

#include "transform.hpp"

/// ----------------------------- Transform ----------------------------------
bool
vl::Transform::isIdentity(void) const
{
	return(position == Ogre::Vector3::ZERO && quaternion == Ogre::Quaternion::IDENTITY);
}

void
vl::Transform::setIdentity(void)
{
	quaternion = Ogre::Quaternion::IDENTITY;
	position = Ogre::Vector3::ZERO;
}

bool
vl::Transform::isPositionZero(void) const
{
	return(position == Ogre::Vector3::ZERO);
}

void
vl::Transform::setPositionZero(void)
{
	position = Ogre::Vector3::ZERO;
}

bool
vl::Transform::isRotationIdentity(void) const
{
	return(quaternion == Ogre::Quaternion::IDENTITY);
}

void 
vl::Transform::setRotationIdentity(void)
{
	quaternion = Ogre::Quaternion::IDENTITY;
}

void
vl::Transform::invert(void)
{
	// Transform needs to have the property that inv(T)*T = I and T*inv(T) = I
	// so the position vector needs to be multiplied with inverse quaternion
	quaternion = quaternion.Inverse();
	position = quaternion*(-position);
}

Ogre::Vector3 
vl::Transform::rotate(Ogre::Vector3 const &v) const
{
	return quaternion*v;
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

vl::Transform &
vl::Transform::operator*=(Ogre::Vector3 const &v)
{
	position *= v;
	return *this;
}

vl::Transform &
vl::Transform::operator*=(vl::scalar s)
{
	position *= s;
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
vl::operator-(vl::Transform const &t)
{
	return t.inverted();
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


Ogre::Vector3
vl::operator*( vl::Transform const &t, Ogre::Vector3 const &v)
{
	return t.quaternion * v + t.position;
}

Ogre::Quaternion
vl::operator*( vl::Transform const &t, Ogre::Quaternion const &q)
{
	return t.quaternion*q;
}

bool
vl::operator==(vl::Transform const &t1, vl::Transform const &t2)
{
	return(t1.position == t2.position && t1.quaternion == t2.quaternion);
}
