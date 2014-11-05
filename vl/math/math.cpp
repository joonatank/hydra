/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-03
 *	@file math/math.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

#include "math.hpp"

/// -------------------------------- Global ----------------------------------
void 
vl::getEulerAngles( Ogre::Quaternion const &q, Ogre::Radian &x, Ogre::Radian &y, Ogre::Radian &z )
{
	vl::scalar test = q.x*q.y + q.z*q.w;
	if (test > 0.5-vl::EPSILON)
	{ // singularity at north pole
		y = 2 * atan2(q.x,q.w);
		z = scalar(M_PI/2);
		x = 0;
		return;
	}
	if (test < -0.5+vl::EPSILON)
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

Ogre::Quaternion
vl::lookAt(Ogre::Vector3 const &target, Ogre::Vector3 const &current, Ogre::Vector3 const &eye, Ogre::Vector3 const &up, bool yawFixed)
{
	// turn vectors into unit vectors 
	Ogre::Vector3 currentDir = (current - eye);
	Ogre::Vector3 targetDir = (target - eye);
	currentDir.normalise();
	targetDir.normalise();

	Quaternion pointToTarget;
    if(yawFixed)
    {
		// Calculate the quaternion for rotate local Z to target direction
		Vector3 xVec = up.crossProduct(targetDir);
		xVec.normalise();
		Vector3 yVec = targetDir.crossProduct(xVec);
		yVec.normalise();
		Quaternion unitZToTarget = Quaternion(xVec, yVec, targetDir);

		if(currentDir == Vector3::NEGATIVE_UNIT_Z)
		{
			// Special case for avoid calculate 180 degree turn
			pointToTarget =
				Quaternion(-unitZToTarget.y, -unitZToTarget.z, unitZToTarget.w, unitZToTarget.x);
		}
        else
        {
			// Calculate the quaternion for rotate local direction to target direction
			Quaternion localToUnitZ = currentDir.getRotationTo(Vector3::UNIT_Z);
			pointToTarget = unitZToTarget * localToUnitZ;
        }
    }
	else
	{
		if ((currentDir+targetDir).squaredLength() < 0.00005f)
		{
			// Oops, a 180 degree turn (infinite possible rotation axes)
			// Default to yaw i.e. use current UP
			pointToTarget = Quaternion(0, 0, 1, 0);
		}
		else
		{
			// Derive shortest arc to new direction
			pointToTarget = currentDir.getRotationTo(targetDir);
		}
	}	

	return pointToTarget;
}
