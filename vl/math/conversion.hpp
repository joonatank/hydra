/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-03
 *
 *	Functions to convert math objects from one library to another.
 *	Ogre Math is our main math library.
 *	
 *	VMMLib <-> Ogre, added 2010-03
 *	VRPN -> Ogre, added 2010-12
 *
 *	TODO add conversion from bullet types
 */
#ifndef VL_MATH_CONVERSION_HPP
#define VL_MATH_CONVERSION_HPP

#include "math/math.hpp"

#include <vmmlib/vector.hpp>
#include <vmmlib/math.hpp>
#include <vmmlib/quaternion.hpp>

#include <OGRE/OgreVector3.h>
#include <OGRE/OgreQuaternion.h>
#include <OGRE/OgreMatrix4.h>
#include <OGRE/OgreColourValue.h>

// VRPN math types
#include <vrpn_Types.h>
#include <quat.h>

namespace vl
{

namespace math
{
	inline 
	vmml::mat4d convert( Ogre::Matrix4 const &mat )
	{
		vmml::mat4d m;
		for( size_t i = 0; i < 4; i++ )
		{
			for( size_t j = 0; j < 4; j++ )
			{ m(i, j) = mat[i][j]; }
		}
		return m;
	}

	inline 
	vmml::vec3d convert( Ogre::Vector3 const &v )
	{
		return vmml::vec3d( v.x, v.y, v.z );
	}
	
	inline
	vmml::quaterniond convert( Ogre::Quaternion const &q )
	{
		return vmml::quaterniond( q.x, q.y, q.z, q.w );
	}

	inline
	Ogre::Matrix4 convert( vmml::mat4d const &mat )
	{
		Ogre::Matrix4 m;
		for( size_t i = 0; i < 4; i++ )
		{
			for( size_t j = 0; j < 4; j++ )
			{ m[i][j] = scalar(mat(i, j)); }
		}
		return m;
	}

	inline
	Ogre::Vector3 convert( vmml::vec3d const &v )
	{
		return Ogre::Vector3( scalar(v.x()), scalar(v.y()), scalar(v.z()) );
	}

	inline
	Ogre::Quaternion convert( vmml::quaterniond const &q )
	{
		return Ogre::Quaternion( scalar(q.w()), scalar(q.x()), scalar(q.y()), scalar(q.z()) );
	}

	// Colour conversions
	/*
	inline
	Ogre::ColourValue convert( vl::colour const &col )
	{
		return Ogre::ColourValue( col.r(), col.g(), col.b(), col.a() );
	}

	inline
	vl::colour convert( Ogre::ColourValue const &col )
	{
		return vl::colour( col.r, col.g, col.b, col.a );
	}
	*/
	inline
	vl::angle convert( Ogre::Radian const &rad )
	{
		return rad.valueRadians();
	}

	inline
	Ogre::Radian convert( vl::angle const &rad )
	{
		return Ogre::Radian(rad);
	}


	// Convert from vrpn_float
	inline
	Ogre::Vector3 convert_vec( vrpn_float64 const *vec )
	{
		scalar x = scalar(vec[Q_X]);
		scalar y = scalar(vec[Q_Y]);
		scalar z = scalar(vec[Q_Z]);
		return Ogre::Vector3( x, y, z );
	}

	inline
	Ogre::Quaternion convert_quat( vrpn_float64 const *quat )
	{
		scalar x = scalar(quat[Q_X]);
		scalar y = scalar(quat[Q_Y]);
		scalar z = scalar(quat[Q_Z]);
		scalar w = scalar(quat[Q_W]);
		return Ogre::Quaternion( w, x, y, z ); 
	}

}	// namespace math

}	// namespace vl

#endif
