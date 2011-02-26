/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-02
 *
 *	@file distrib_settings.cpp
 *	Classes for distributing Environment Settings and Project settings
 *	using ByteStream
 */

#include "distrib_settings.hpp"

#include <iostream>
#include "settings.hpp"

vl::SettingsByteData::SettingsByteData( void )
{
// 	std::cout << "vl::SettingsByteData::SettingsByteData" << std::endl;
}

vl::SettingsByteData::~SettingsByteData( void )
{
// 	std::cout << "vl::SettingsByteData::~SettingsByteData" << std::endl;
}

void
vl::SettingsByteData::read( char* mem, vl::msg_size size )
{
// 	std::cout << "vl::SettingsByteData::read" << std::endl;
	assert( mem );
	if( 0 == size )
	{ return; }

	assert( _data.size() >= size );

	::memcpy( mem, &_data[0], size );
	_data.erase( _data.begin(), _data.begin()+size );
}

void
vl::SettingsByteData::write( const char* mem, vl::msg_size size )
{
// 	std::cout << "vl::SettingsByteData::write" << std::endl;
	assert( mem );
	if( 0 == size )
	{ return; }

	size_t pos = _data.size();
	_data.resize( _data.size() + size );
	::memcpy( &_data[pos], mem, size);
}

void
vl::SettingsByteData::copyToMessage( vl::cluster::Message* msg )
{
// 	std::cout << "vl::SettingsByteData::copyToMessage" << std::endl;
	assert(msg);

	// TODO copy settings type

	assert( _data.size() < msg_size(-1) );
	msg_size size = _data.size();
	msg->write(size);
	msg->write( &_data[0], size );
}


void
vl::SettingsByteData::copyFromMessage( vl::cluster::Message* msg )
{
// 	std::cout << "vl::SettingsByteData::copyFromMessage" << std::endl;
	assert(msg);

	// TODO copy settings type

	msg_size size;
	// Check that there is more data than the size
	assert( msg->size() > sizeof(size)-1 );
	msg->read(size);
	assert( msg->size() > size-1 );
	_data.resize(size);

	msg->read( &_data[0], size );
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator<<( vl::cluster::ByteStream& msg, vl::EnvSettingsRefPtr const &env )
{
// 	std::cout << "vl::cluster::operator<< ( vl::cluster::ByteStream& msg, vl::EnvSettingsRefPtr env )" << std::endl;
	msg << env->getVerbose() << env->getCameraRotationAllowed()
		<< env->getMaster() << env->getSlaves()
		<< env->getWalls() << env->getStereo()<< env->getIPD() << env->getLogDir();

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>( vl::cluster::ByteStream& msg, vl::EnvSettingsRefPtr &env )
{
// 	std::cout << "vl::cluster::operator>>( vl::cluster::ByteStream& msg, vl::EnvSettingsRefPtr env )" << std::endl;
	uint32_t rot_allowed(-1);
	vl::CFG stereo = vl::OFF;
	double ipd = 0;
	std::string log_dir;
	bool verbose;
	msg >> verbose >> rot_allowed >> env->getMaster()
		>> env->getSlaves() >> env->getWalls()
		>> stereo >> ipd >> log_dir;
	env->setCameraRotationAllowed( rot_allowed );
	env->setStereo(stereo);
	env->setIPD(ipd);
	env->setLogDir(log_dir);
	env->setVerbose(verbose);

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator<<( vl::cluster::ByteStream& msg, const vl::EnvSettings::Node& node )
{
	msg << node.name << node.windows;

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>( vl::cluster::ByteStream& msg, vl::EnvSettings::Node& node )
{
	msg >> node.name >> node.windows;

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator<<( vl::cluster::ByteStream& msg, const vl::EnvSettings::Wall& wall )
{
	msg << wall.name << wall.bottom_left << wall.bottom_right
		<< wall.top_left;

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>( vl::cluster::ByteStream& msg, vl::EnvSettings::Wall& wall )
{
	msg >> wall.name >> wall.bottom_left >> wall.bottom_right
		>> wall.top_left;

	return msg;

}

template<>
vl::cluster::ByteStream &
vl::cluster::operator<<( vl::cluster::ByteStream& msg, const vl::EnvSettings::Window& window )
{
	msg << window.name << window.channel << window.x << window.y << window.h
		<< window.w;

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>( vl::cluster::ByteStream& msg, vl::EnvSettings::Window& window )
{
	msg >> window.name >> window.channel >> window.x >> window.y >> window.h
		>> window.w;

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator<<( vl::cluster::ByteStream& msg, vl::EnvSettings::Channel const &chan )
{
	msg << chan.name << chan.wall_name;

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>( vl::cluster::ByteStream& msg, vl::EnvSettings::Channel &chan )
{
	msg >> chan.name >> chan.wall_name;

	return msg;
}



template<>
vl::cluster::ByteStream &
vl::cluster::operator<<( vl::cluster::ByteStream& msg, const vl::Settings& settings )
{
	msg << settings.getProjectSettings() << settings.getAuxilarySettings();

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>( vl::cluster::ByteStream& msg, vl::Settings& settings )
{
	vl::ProjSettings proj;
	msg >> proj;
	settings.setProjectSettings(proj);
	size_t size;
	msg >> size;
	for( size_t i = 0; i < size; ++i )
	{
		vl::ProjSettings aux;
		msg >> aux;
		settings.addAuxilarySettings(aux);
	}

	return msg;
}


template<>
vl::cluster::ByteStream &
vl::cluster::operator<<( vl::cluster::ByteStream& msg, vl::ProjSettings const &proj )
{
	msg << proj.getFile();
	vl::ProjSettings::Case const &cas = proj.getCase();
	msg << proj.getName() << cas.getNscenes();
	for( size_t i = 0; i < cas.getNscenes(); ++i )
	{
		msg << cas.getScene(i);
	}

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>( vl::cluster::ByteStream& msg, vl::ProjSettings &proj )
{
	std::string file;
	std::string name;
	size_t size;
	msg >> file >> name >> size;

	proj.setFile(file);
	proj.setName(name);

	vl::ProjSettings::Case &cas = proj.getCase();
	assert( cas.getNscenes() == 0 );
	for( size_t i = 0; i < size; ++i )
	{
		vl::ProjSettings::Scene scene;
		msg >> scene;
		cas.addScene(scene);
	}

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator<<( vl::cluster::ByteStream& msg, const vl::ProjSettings::Scene &scene )
{
	msg << scene.getName() << scene.getUse() << scene.getFile();

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>( vl::cluster::ByteStream& msg, vl::ProjSettings::Scene &scene )
{
	std::string name;
	std::string file;
	bool use;
	msg >> name >> use >> file;
	scene.setName(name);
	scene.setUse(use);
	scene.setFile(file);

	return msg;
}
