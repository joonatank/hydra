#ifndef VL_MATH_CONVERSION_HPP
#define VL_MATH_CONVERSION_HPP

#include <vmmlib/matrix.hpp>
#include <vmmlib/vector.hpp>
#include <vmmlib/quaternion.hpp>

#include <OGRE/OgreVector3.h>
#include <OGRE/OgreQuaternion.h>
#include <OGRE/OgreMatrix4.h>

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
		return vmml::quaterniond( q.w, q.x, q.y, q.z );
	}

	inline
	Ogre::Matrix4 convert( vmml::mat4d const &mat )
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
	Ogre::Vector3 convert( vmml::vec3d const &v )
	{
		return Ogre::Vector3( v.x(), v.y(), v.z() );
	}

	inline
	Ogre::Quaternion convert( vmml::quaterniond const &q )
	{
		return Ogre::Quaternion( q.w(), q.x(), q.y(), q.z() );
	}

}	// namespace math

}	// namespace vl

#endif
