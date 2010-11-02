#ifndef EQ_OGRE_SERIALIZE_HELPERS_HPP
#define EQ_OGRE_SERIALIZE_HELPERS_HPP

#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>

#include <OGRE/OgreVector3.h>
#include <OGRE/OgreQuaternion.h>

namespace eqOgre
{
	
inline
eq::net::DataOStream &operator<<( Ogre::Vector3 const &v, eq::net::DataOStream &os )
{
	os << v[0] << v[1] << v[2];
	return os;
}

inline
eq::net::DataIStream &operator>>( Ogre::Vector3 &v, eq::net::DataIStream &is )
{
	is >> v[0] >> v[1] >> v[2];
	return is;
}

inline
eq::net::DataOStream &operator<<( Ogre::Quaternion const &q, eq::net::DataOStream &os )
{
	os << q[0] << q[1] << q[2] << q[3];
	return os;
}


inline
eq::net::DataIStream &operator>>( Ogre::Quaternion &q, eq::net::DataIStream &is )
{
	is >> q[0] >> q[1] >> q[2] >> q[3];
	return is;
}

}	// namespace eqOgre

#endif