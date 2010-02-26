#ifndef EQOGRE_UTILITY_HPP
#define EQOGRE_UTILITY_HPP

#include <iostream>
#include <vector>

#define NOT_IMPLEMENTED() std::cerr << "NOT IMPLEMENTED" << std::endl;

#define NOT_TESTED() std::cerr << "NOT TESTED" << std::endl;

#include <OGRE/OgrePrerequisites.h>
#include <OGRE/OgreVector3.h>
#include <OGRE/OgreQuaternion.h>

#include <eq/net/dataOStream.h>
#include <eq/net/dataIStream.h>

/*
eq::net::DataOStream &
operator<<( eq::net::DataOStream& os, Ogre::String const& str );

eq::net::DataIStream &
operator>>( eq::net::DataIStream& is, Ogre::String& str );

eq::net::DataOStream &
operator<<( eq::net::DataOStream& os, Ogre::Quaternion const& q );

eq::net::DataIStream &
operator>>( eq::net::DataIStream& is, Ogre::Quaternion& q );

eq::net::DataOStream &
operator<<( eq::net::DataOStream& os, Ogre::Vector3 const& v );

eq::net::DataIStream &
operator>>( eq::net::DataIStream& is, Ogre::Vector3& v );

eq::net::DataOStream &
operator<<( eq::net::DataOStream& os, Ogre::Real const& d );

eq::net::DataIStream &
operator>>( eq::net::DataIStream& is, Ogre::Real& d );
*/

/*
template< typename T >
eq::net::DataOStream &
operator<<( eq::net::DataOStream& os, std::vector<T> const &vec )
{
	os << vec.size();
	for( size_t i = 0; i < vec.size(); i++ )
	{
		os << vec.at(i);
	}

	return os;
}

// Vector deserializer, overwrites the input vector.
// So it operates like any other deserializer operator on objects.
template< typename T >
eq::net::DataIStream &
operator>>( eq::net::DataIStream& is, std::vector<T> &vec )
{
	size_t size;
	is >> size;
	vec.resize( size );
	for( size_t i = 0; i < size; i++ )
	{
	//	T t;
		is >> vec.at(i);
	}

	return is;
}
*/

template< typename T, typename H >
eq::net::DataOStream &
operator<<( eq::net::DataOStream& os, std::map<T, H> const &map )
{
	os << map.size();
	typename std::map<T, H>::const_iterator iter = map.begin();
	for( ; iter != map.end(); ++iter )
	{
		os << iter->first << iter->second;
	}
	return os;
}

// Map deserializer, overwrites the map.
template< typename T, typename H >
eq::net::DataIStream &
operator>>( eq::net::DataIStream& is, std::map<T, H> &map )
{
	size_t size;
	is >> size;
	map.clear();
	typename std::map<T, H>::iterator iter = map.begin();
	for( size_t i = 0; i < size; i++ )
	{
		std::pair<T, H> pair;
		is >> pair.first >> pair.second;
		
		iter = map.insert( iter, pair );
	}

	return is;
}
// Possibly add serialization of
// eqOgre::SceneNode, eqOgre::Entity, eqOgre::Light, eqOgre::Camera
// Interesting question though is what we are serializing here,
// name or typeID would probably be the most usefull.
// And only the owner has the right to register/deregister
// (eg. create, destroy) these.
// And they themselves only to change their parameters.
/*
 * Can't be implemented strait forwardly, 
 * because we need to find existing object and pass a pointer to it, not copy it.
 * Using smart pointers would propably work though.
eq::net::DataOStream& operator<<( eq::net::DataOStream& os, Ogre::MovableObject const& obj )
{
	os << obj->getName();
}

eq::net::DataIStream& operator>>( eq::net::DataIStream& is, Ogre::MovableObject & obj )
{
	Ogre::String str;
	is >> str;

	if( eqOgre::Entity *ent = dynamic_cast<eqOgre::Entity *>(obj) )
	{

	}
	else if( eqOgre::Camera *cam = dynamic_cast<eqOgre::Camera *>(obj) )
	{

	}
	else if( eqOgre::Light *light = dynamic_cast<eqOgre::Light *>(obj) )
	{

	}

}

eq::net::DataOStream& operator<<( eq::net::DataOStream& os, Ogre::SceneNode const& node )
{
	os << node->getName();
}

eq::net::DataIStream& operator>>( eq::net::DataIStream& is, Ogre::SceneNode & node )
{

}
*/


#endif
