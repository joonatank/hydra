#ifndef VL_MATH_CONVERSION_HPP
#define VL_MATH_CONVERSION_HPP

#include "math/math.hpp"

#include <OGRE/OgreVector3.h>
#include <OGRE/OgreQuaternion.h>
#include <OGRE/OgreMatrix4.h>
#include <OGRE/OgreColourValue.h>

namespace vl
{

namespace math
{
	inline 
	vl::matrix convert( Ogre::Matrix4 const &mat )
	{
		vl::matrix m;
		for( size_t i = 0; i < 4; i++ )
		{
			for( size_t j = 0; j < 4; j++ )
			{ m(i, j) = mat[i][j]; }
		}
		return m;
	}

	inline 
	vl::vector convert( Ogre::Vector3 const &v )
	{
		return vl::vector( v.x, v.y, v.z );
	}
	
	inline
	vl::quaternion convert( Ogre::Quaternion const &q )
	{
		return vl::quaternion( q.x, q.y, q.z, q.w );
	}

	inline
	Ogre::Matrix4 convert( vl::matrix const &mat )
	{
		Ogre::Matrix4 m;
		for( size_t i = 0; i < 4; i++ )
		{
			for( size_t j = 0; j < 4; j++ )
			{ m[i][j] = mat(i, j); }
		}
		return m;
	}

	inline
	Ogre::Vector3 convert( vl::vector const &v )
	{
		return Ogre::Vector3( v.x(), v.y(), v.z() );
	}

	inline
	Ogre::Quaternion convert( vl::quaternion const &q )
	{
		return Ogre::Quaternion( q.w(), q.x(), q.y(), q.z() );
	}

	// Colour conversions
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
}	// namespace math

}	// namespace vl

#endif
