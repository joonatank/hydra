/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2011-02
 *
 *	Classes for distributing Environment Settings and Project settings
 *	using ByteStream
 */

#ifndef VL_DISTRIB_SETTINGS_HPP
#define VL_DISTRIB_SETTINGS_HPP

// Base class
#include "cluster/message.hpp"

// Which classes these operate
#include "base/envsettings.hpp"
#include "base/projsettings.hpp"

namespace vl
{

enum SETTTINGS_TYPE
{
	SET_ENV,	// Environment settings
	SET_PROJ,	// Project settings
};

class SettingsByteData : public vl::cluster::ByteData
{
public :
	SettingsByteData( void );

	~SettingsByteData( void );

	virtual void read( char *mem, msg_size size );
	virtual void write( char const *mem, msg_size size );

	virtual void open( void ) {}
	virtual void close( void ) {}

	virtual void copyToMessage( vl::cluster::Message *msg );
	virtual void copyFromMessage( vl::cluster::Message *msg );

private :
	std::vector<char> _data;
	SETTTINGS_TYPE _type;

};	// class EnvSettingsByteCopy

namespace cluster
{

template<>
ByteStream &operator<<( ByteStream &msg, vl::EnvSettingsRefPtr const &env );

template<>
ByteStream &operator>>( ByteStream &msg, vl::EnvSettingsRefPtr &env );

template<>
ByteStream &operator<<( ByteStream &msg, vl::EnvSettings::Node const &node );

template<>
ByteStream &operator>>( ByteStream &msg, vl::EnvSettings::Node &node );

template<>
ByteStream &operator<<( ByteStream &msg, vl::EnvSettings::Wall const &wall );

template<>
ByteStream &operator>>( ByteStream &msg, vl::EnvSettings::Wall &wall );

template<>
ByteStream &operator<<( ByteStream &msg, vl::EnvSettings::Window const &window );

template<>
ByteStream &operator>>( ByteStream &msg, vl::EnvSettings::Window &window );



template<>
ByteStream &operator<<( ByteStream &msg, vl::ProjSettingsRefPtr const &proj );

template<>
ByteStream &operator>>( ByteStream &msg, vl::ProjSettingsRefPtr &proj );

}	// namespace cluster

}	// namespace vl

#endif // VL_DISTRIB_SETTINGS_HPP