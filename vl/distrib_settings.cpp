/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2011-02
 *
 *	Classes for distributing Environment Settings and Project settings
 *	using ByteStream
 */

#include "distrib_settings.hpp"

#include <iostream>

vl::SettingsByteData::SettingsByteData( void )
{
	std::cout << "vl::SettingsByteData::SettingsByteData" << std::endl;
}

vl::SettingsByteData::~SettingsByteData( void )
{
	std::cout << "vl::SettingsByteData::~SettingsByteData" << std::endl;
}

void
vl::SettingsByteData::read( char* mem, vl::msg_size size )
{
	std::cout << "vl::SettingsByteData::read" << std::endl;
}

void
vl::SettingsByteData::write( const char* mem, vl::msg_size size )
{
	std::cout << "vl::SettingsByteData::write" << std::endl;
}

void
vl::SettingsByteData::copyToMessage( vl::cluster::Message* msg )
{
	std::cout << "vl::SettingsByteData::copyToMessage" << std::endl;
}


void
vl::SettingsByteData::copyFromMessage( vl::cluster::Message* msg )
{
	std::cout << "vl::SettingsByteData::copyFromMessage" << std::endl;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator<<( vl::cluster::ByteStream& msg, vl::EnvSettingsRefPtr const &env )
{
	std::cout << "vl::cluster::operator<< ( vl::cluster::ByteStream& msg, vl::EnvSettingsRefPtr env )" << std::endl;
	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>( vl::cluster::ByteStream& msg, vl::EnvSettingsRefPtr &env )
{
	std::cout << "vl::cluster::operator>>( vl::cluster::ByteStream& msg, vl::EnvSettingsRefPtr env )" << std::endl;
	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator<<( vl::cluster::ByteStream& msg, vl::ProjSettingsRefPtr const &proj )
{
	std::cout << "vl::cluster::operator<<( vl::cluster::ByteStream& msg, vl::ProjSettingsRefPtr proj )" << std::endl;
	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>( vl::cluster::ByteStream& msg, vl::ProjSettingsRefPtr &proj )
{
	std::cout << "vl::cluster::operator>>( vl::cluster::ByteStream& msg, vl::ProjSettingsRefPtr proj )" << std::endl;
	return msg;
}
