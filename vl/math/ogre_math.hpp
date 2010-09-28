#ifndef VL_MATH_OGRE_MATH_HPP
#define VL_MATH_OGRE_MATH_HPP

#include <OgreVector3.h>

namespace Ogre
{
	bool equal( Ogre::Real a, Ogre::Real b, Ogre::Real epsilon )
	{
		return (a < b + epsilon && a + epsilon > b );
	}
	
	bool equal( Vector3 const &a, Vector3 const &b, const Ogre::Real epsilon )
	{
		for( size_t i = 0; i < 3; ++i )
		{
			if( !equal( a[i], b[i], epsilon) )
			{ return false; }
		}
		return true;
	}

	bool equal( Quaternion const &a, Quaternion const &b, const Ogre::Real epsilon )
	{
		for( size_t i = 0; i < 4; ++i )
		{
			if( !equal( a[i], b[i], epsilon) )
			{ return false; }
		}
		return true;
	}
}

#endif
