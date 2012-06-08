/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011/10 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-02
 *	@file distrib_settings.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

#include "distrib_settings.hpp"

#include <iostream>
#include "settings.hpp"

vl::SettingsByteData::SettingsByteData( void )
{}

vl::SettingsByteData::~SettingsByteData( void )
{}

void
vl::SettingsByteData::read( char* mem, vl::msg_size size )
{
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
	assert(msg);

	assert( _data.size() < msg_size(-1) );
	msg_size size = _data.size();
	msg->write(size);
	msg->write( &_data[0], size );
}


void
vl::SettingsByteData::copyFromMessage( vl::cluster::Message* msg )
{
	assert(msg);

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
vl::cluster::operator<<( vl::cluster::ByteStream& msg, vl::config::EnvSettingsRefPtr const &env )
{
	msg << env->getLogLevel() << env->getCameraRotationAllowed()
		<< env->getMaster() << env->getSlaves()
		<< env->getWalls() << env->getIPD() << env->getLogDir();

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>( vl::cluster::ByteStream& msg, vl::config::EnvSettingsRefPtr &env )
{
	uint32_t rot_allowed(-1);
	double ipd = 0;
	std::string log_dir;
	vl::config::LogLevel log_level;
	msg >> log_level >> rot_allowed >> env->getMaster()
		>> env->getSlaves() >> env->getWalls()
		>> ipd >> log_dir;

	env->setCameraRotationAllowed( rot_allowed );
	env->setIPD(ipd);
	env->setLogDir(log_dir);
	env->setLogLevel(log_level);

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator<<( vl::cluster::ByteStream& msg, vl::config::Node const &node )
{
	msg << node.name << node.windows;

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>( vl::cluster::ByteStream& msg, vl::config::Node& node )
{
	msg >> node.name >> node.windows;

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator<<( vl::cluster::ByteStream& msg, const vl::Wall& wall )
{
	msg << wall.name << wall.bottom_left << wall.bottom_right
		<< wall.top_left;

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>( vl::cluster::ByteStream& msg, vl::Wall& wall )
{
	msg >> wall.name >> wall.bottom_left >> wall.bottom_right
		>> wall.top_left;

	return msg;

}

template<>
vl::cluster::ByteStream &
vl::cluster::operator<<( vl::cluster::ByteStream& msg, const vl::config::Window& window )
{
	msg << window.name << window.get_channels() << window.rect
		<< window.n_display << window.stereo_type << window.nv_swap_sync 
		<< window.nv_swap_group << window.nv_swap_barrier << window.vert_sync
		<< window.renderer << window.input_handler;

	msg << window.params.size();
	for(NamedParamList::const_iterator iter = window.params.begin(); 
		iter != window.params.end(); ++iter)
	{
		msg << iter->first << iter->second;
	}

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>( vl::cluster::ByteStream& msg, vl::config::Window& window )
{
	msg >> window.name >> window.get_channels() >> window.rect
		>> window.n_display >> window.stereo_type >> window.nv_swap_sync
		>> window.nv_swap_group >> window.nv_swap_barrier >> window.vert_sync
		>> window.renderer >> window.input_handler;

	size_t params_size;
	msg >> params_size;
	for(size_t i = 0; i < params_size; ++i)
	{
		std::string param_name, param_value;
		msg >> param_name >> param_value;
		window.params[param_name] = param_value;
	}

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator<<( vl::cluster::ByteStream& msg, vl::config::Channel const &chan )
{
	msg << chan.name << chan.wall_name << chan.area << chan.background_colour;

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>( vl::cluster::ByteStream& msg, vl::config::Channel &chan )
{
	msg >> chan.name >> chan.wall_name >> chan.area >> chan.background_colour;

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator<<(vl::cluster::ByteStream &msg, vl::config::Renderer const &rend)
{
	msg << rend.type << rend.projection << rend.hardware_gamma;

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>(vl::cluster::ByteStream &msg, vl::config::Renderer &rend)
{
	msg >> rend.type >> rend.projection >> rend.hardware_gamma;

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator<<(vl::cluster::ByteStream &msg, vl::config::Projection const &projection)
{
	msg << projection.type << projection.perspective_type << projection.fov 
		<< projection.horizontal << projection.head_x << projection.head_y << projection.head_z 
		<< projection.modify_transformations;

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>(vl::cluster::ByteStream &msg, vl::config::Projection &projection)
{
	msg >> projection.type >> projection.perspective_type >> projection.fov
		>> projection.horizontal >> projection.head_x >> projection.head_y >> projection.head_z 
		>> projection.modify_transformations;

	return msg;
}



template<>
vl::cluster::ByteStream &
vl::cluster::operator<<( vl::cluster::ByteStream& msg, const vl::Settings& settings )
{
	msg << settings.getProjectSettings() << settings.getAuxilarySettings();

	return msg;
}

/// @todo the only information needed from these is paths, so we might as well
/// just send them without serializing the projects.
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
	msg << proj.getFile() << proj.getName();

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>( vl::cluster::ByteStream& msg, vl::ProjSettings &proj )
{
	std::string file;
	std::string name;
	msg >> file >> name;

	proj.setFile(file);
	proj.setName(name);

	return msg;
}
