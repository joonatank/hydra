/**
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2013-06
 *	@file object_interface.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

#include "object_interface.hpp"

#include "math/math.hpp"
#include "base/exceptions.hpp"

void 
vl::ObjectInterface::transform(Ogre::Matrix4 const &m)
{
	if(m != Ogre::Matrix4::IDENTITY)
	{
		Ogre::Vector3 s;
		Transform t;
		m.decomposition(t.position, s, t.quaternion);

		transform(t);
		scale(s);
	}
}

void 
vl::ObjectInterface::setTransform(Ogre::Matrix4 const &m)
{
	Ogre::Vector3 s, pos;
	Ogre::Quaternion q;
	m.decomposition(pos, s, q);
	
	setOrientation(q);
	setPosition(pos);
	setScale(s);
}

void 
vl::ObjectInterface::setTransform(vl::Transform const &trans, vl::ObjectInterfacePtr reference)
{
	vl::Transform wt = getParentWorldTransform().inverted();

	setTransform(wt * reference->getWorldTransform() * trans);
}

vl::Transform 
vl::ObjectInterface::getTransform(vl::ObjectInterfacePtr reference) const
{
	vl::Transform wt = getParentWorldTransform().inverted();

	return reference->getWorldTransform() * wt * getTransform();
}

vl::Transform
vl::ObjectInterface::getParentWorldTransform(void) const
{
	Transform const &wt = getWorldTransform();
	Transform const &t = getTransform();
	Transform diff = wt*t.inverted();

	return diff;
}


void 
vl::ObjectInterface::transform(vl::Transform const &trans)
{
	if(!trans.isIdentity())
	{
		setTransform(getTransform()*trans);
	}
}

void 
vl::ObjectInterface::translate(Ogre::Vector3 const &v, vl::ObjectInterfacePtr reference)
{
	/// World space
	if(!reference)
	{
		translate(v, TS_WORLD);
	}
	/// Local space
	else if(reference == this)
	{
		translate(v, TS_LOCAL);
	}
	/// Reference space
	else
	{
		translate(reference->getWorldTransform().quaternion*v, TS_WORLD);
	}
}

/// @todo change setPosition to setTransform
void 
vl::ObjectInterface::translate(Ogre::Vector3 const &v, vl::TransformSpace space)
{
	switch(space)
	{
	case TS_LOCAL:
		setPosition(getTransform().position + getTransform().rotate(v));
		break;

	case TS_PARENT:
		setPosition(getTransform().position + v);
		break;

	case TS_WORLD:
	{
		// Seems to translate stuff in world properly
		Transform wt = getParentWorldTransform().inverted();
		Ogre::Vector3 mov = getTransform().position + wt.quaternion*v;
		setPosition(mov);
	}
	break;

	default :
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Unknown transform space"));
	}
}

void 
vl::ObjectInterface::translate(Ogre::Vector3 const &v)
{
	translate(v, TS_LOCAL);
}

void
vl::ObjectInterface::rotateAround(Ogre::Quaternion const &q, vl::ObjectInterfacePtr reference)
{
	/// @todo we should not throw here instead 
	/// if we don't have reference rotate around world origin
	/// if reference == this we should rotate around this object
	if(!reference)
	{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Rotate around without the point.")); }
	if(reference == this)
	{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Rotate around with this as point.")); }

	/// With references, this function is rotate around a point
	/// the matrix to represent such a rotation is T = inv(P)*R*P
	/// where P is translate matrix for the point and R is the rotation matrix
	/// this matrix can be represented with [Tq, Tv] = [Rq, Rq*p - p]
	/// where Tq is the quaternion part of T and Tv is the vector part
	/// Rq is the quaternion representation of R and p is the vector of P
	/// seems like this is for left handed coordinate system, switch inv(P) and P for right handed.

	// Um seems like the rotate around doesn't not work as expected when
	// using multiple rotations. Though the usefulness of this feature is depatable.
	

	vl::Transform t;
	/// @todo this is incorrect
	vl::Transform ref_world = reference->getWorldTransform();
	//Transform inv_world = getWorldTransform();
	//inv_world.invert();
	t.quaternion = q;
	t.position = ref_world.position - q*ref_world.position;
	setTransform(t*getTransform()*t.inverted());
}

void
vl::ObjectInterface::rotate(Ogre::Quaternion const &q, vl::TransformSpace space)
{
	switch(space)
	{
		case TS_LOCAL:
		{
			Transform t(q);
			setTransform(getTransform()*t);
		}
		break;

		case TS_PARENT:
		{
			Transform t(q);
			setTransform(t*getTransform());
		}
		break;

		case TS_WORLD:
		{
			Transform const &world = getWorldTransform();
			Transform t;
			t.position = getTransform().position;
			t.quaternion = getTransform().quaternion * world.quaternion.Inverse() * q *world.quaternion;
			//t.position = getWorldTransform().position - q*getWorldTransform().position;
			setTransform(t);
		}
		break;

	}
}

void
vl::ObjectInterface::rotate(Ogre::Quaternion const &q)
{
	rotate(q, TS_LOCAL);
}

void
vl::ObjectInterface::lookAt(Ogre::Vector3 const &dir)
{
	Ogre::Quaternion q = vl::lookAt(dir, getDirection(), getTransform().position);
	rotate(q);
}

Ogre::Vector3
vl::ObjectInterface::getDirection(void) const
{
	return getTransform().quaternion * Ogre::Vector3::NEGATIVE_UNIT_Z;
}

void
vl::ObjectInterface::setDirection(Ogre::Vector3 const &dir, Ogre::Vector3 const &localDirectionVector, Ogre::Vector3 const &upAxis, bool yawFixed)
{
	Ogre::Vector3 targetDir = dir.normalisedCopy();

	Quaternion targetOrientation;
    if(yawFixed)
    {
		// Calculate the quaternion for rotate local Z to target direction
		Vector3 xVec = upAxis.crossProduct(targetDir);
		xVec.normalise();
		Vector3 yVec = targetDir.crossProduct(xVec);
		yVec.normalise();
		Quaternion unitZToTarget = Quaternion(xVec, yVec, targetDir);

		if(localDirectionVector == Vector3::NEGATIVE_UNIT_Z)
		{
			// Special case for avoid calculate 180 degree turn
			targetOrientation =
				Quaternion(-unitZToTarget.y, -unitZToTarget.z, unitZToTarget.w, unitZToTarget.x);
		}
        else
        {
			// Calculate the quaternion for rotate local direction to target direction
			Quaternion localToUnitZ = localDirectionVector.getRotationTo(Vector3::UNIT_Z);
			targetOrientation = unitZToTarget * localToUnitZ;
        }
    }
	else
	{
		Quaternion const &currentOrient = getWorldTransform().quaternion;

		// Get current local direction relative to world space
		Vector3 currentDir = currentOrient * localDirectionVector;

		if ((currentDir+targetDir).squaredLength() < 0.00005f)
		{
			// Oops, a 180 degree turn (infinite possible rotation axes)
			// Default to yaw i.e. use current UP
			targetOrientation =
				Quaternion(-currentOrient.y, -currentOrient.z, currentOrient.w, currentOrient.x);
		}
		else
		{
			// Derive shortest arc to new direction
			Ogre::Quaternion rotQuat = currentDir.getRotationTo(targetDir);
			targetOrientation = rotQuat * currentOrient;
		}
	}

	setOrientation(targetOrientation);
}
