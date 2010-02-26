//#include "sceneManager.hpp"

#include "utility.hpp"

/*
eq::net::DataOStream &
operator<<( eq::net::DataOStream& os, Ogre::String const& str )
{
	os << str.size();
	for( size_t i = 0; i < str.size(); i++ )
	{
		os << str.at(i);
	}

	return os;
}

eq::net::DataIStream &
operator>>( eq::net::DataIStream& is, Ogre::String& str )
{
	size_t size;
	is >> size;

	str.clear();
	for( size_t i = 0; i < str.size(); i++ )
	{
		char ch;
		is >> ch;
		str.push_back(ch);
	}

	return is;
}

eq::net::DataOStream &
operator<<( eq::net::DataOStream& os, Ogre::Quaternion const& q )
{
	os << q.w << q.x << q.y << q.z;
	return os;
}

eq::net::DataIStream &
operator>>( eq::net::DataIStream& is, Ogre::Quaternion& q )
{
//	Ogre::Real w, x, y, z;
	is >> q.w >> q.x >> q.y >> q.z;
	return is;
}

eq::net::DataOStream &
operator<<( eq::net::DataOStream& os, Ogre::Vector3 const& v )
{
	os <<  v.x << v.y << v.z;
	return os;
}

eq::net::DataIStream &
operator>>( eq::net::DataIStream& is, Ogre::Vector3& v )
{
	is >> v.x >> v.y >> v.z;
	return is;
}

eq::net::DataOStream &
operator<<( eq::net::DataOStream& os, Ogre::Real const& d )
{
	os << d;
	return os;
}

eq::net::DataIStream &
operator>>( eq::net::DataIStream& is, Ogre::Real& d )
{
	is >> d;
	return is;
}
*/
