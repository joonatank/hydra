/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-02
 *
 *	@file distrib_settings.hpp
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

class SettingsByteData : public vl::cluster::ByteData
{
public :
	SettingsByteData( void );

	virtual ~SettingsByteData( void );

	virtual void read( char *mem, msg_size size );
	virtual void write( char const *mem, msg_size size );

	virtual void open( void ) {}
	virtual void close( void ) {}

	virtual void copyToMessage( vl::cluster::Message *msg );
	virtual void copyFromMessage( vl::cluster::Message *msg );

private :
	std::vector<char> _data;

};	// class EnvSettingsByteCopy

namespace cluster
{

template<>
ByteStream &operator<<( ByteStream &msg, vl::config::EnvSettingsRefPtr const &env );

template<>
ByteStream &operator>>( ByteStream &msg, vl::config::EnvSettingsRefPtr &env );

template<>
ByteStream &operator<<( ByteStream &msg, vl::config::Node const &node );

template<>
ByteStream &operator>>( ByteStream &msg, vl::config::Node &node );

template<>
ByteStream &operator<<( ByteStream &msg, vl::Wall const &wall );

template<>
ByteStream &operator>>( ByteStream &msg, vl::Wall &wall );

template<>
ByteStream &operator<<( ByteStream &msg, vl::config::Window const &window );

template<>
ByteStream &operator>>( ByteStream &msg, vl::config::Window &window );

template<>
ByteStream &operator<<( ByteStream &msg, vl::config::Channel const &chan );

template<>
ByteStream &operator>>( ByteStream &msg, vl::config::Channel &chan );

template<>
ByteStream &operator<<(ByteStream &msg, vl::config::Renderer const &rend);

template<>
ByteStream &operator>>(ByteStream &msg, vl::config::Renderer &rend);

template<>
ByteStream &operator<<(ByteStream &msg, vl::config::Projection const &projection);

template<>
ByteStream &operator>>(ByteStream &msg, vl::config::Projection &projection);

template<>
ByteStream &operator<<( ByteStream &msg, vl::Settings const &settings);

template<>
ByteStream &operator>>( ByteStream &msg, vl::Settings &settings );

template<>
ByteStream &operator<<( ByteStream &msg, vl::ProjSettings const &proj );

template<>
ByteStream &operator>>( ByteStream &msg, vl::ProjSettings &proj );

}	// namespace cluster

}	// namespace vl

#endif // VL_DISTRIB_SETTINGS_HPP
