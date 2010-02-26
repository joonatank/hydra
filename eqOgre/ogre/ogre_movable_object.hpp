/*	Joonatan Kuosa
 *	2010-02
 *
 *	This is probably useless class, as we have to inherit from specific
 *	movable type in vl::graph.
 *	TODO don't use.
 *
 *	Using it as interface for getNative in all movableobjects
 */

#ifndef VL_OGRE_MOVABLE_OBJECT_HPP
#define VL_OGRE_MOVABLE_OBJECT_HPP

#include <OGRE/OgreMovableObject.h>

namespace vl
{

namespace ogre
{
	class MovableObject
	{
		public :
			virtual Ogre::MovableObject *getNative( void ) = 0;

	};	// class MovableObject

}	// namespace ogre

}	// namespace vl

#endif
