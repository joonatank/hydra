#ifndef EQ_OGRE_SERIALIZE_HELPERS_HPP
#define EQ_OGRE_SERIALIZE_HELPERS_HPP

#include <co/dataIStream.h>
#include <co/dataOStream.h>

#include <OGRE/OgreVector3.h>
#include <OGRE/OgreQuaternion.h>

namespace eqOgre
{
	
inline
co::DataOStream &operator<<( Ogre::Vector3 const &v, co::DataOStream &os )
{
	os << v[0] << v[1] << v[2];
	return os;
}

inline
co::DataIStream &operator>>( Ogre::Vector3 &v, co::DataIStream &is )
{
	is >> v[0] >> v[1] >> v[2];
	return is;
}

inline
co::DataOStream &operator<<( Ogre::Quaternion const &q, co::DataOStream &os )
{
	os << q[0] << q[1] << q[2] << q[3];
	return os;
}


inline
co::DataIStream &operator>>( Ogre::Quaternion &q, co::DataIStream &is )
{
	is >> q[0] >> q[1] >> q[2] >> q[3];
	return is;
}

}	// namespace eqOgre

#endif