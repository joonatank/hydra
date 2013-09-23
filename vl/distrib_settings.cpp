/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2013 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-02
 *	@file distrib_settings.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

#include "distrib_settings.hpp"

#include <iostream>

template<>
vl::cluster::ByteStream &
vl::cluster::operator<<( vl::cluster::ByteStream& msg, vl::config::EnvSettingsRefPtr const &env )
{
	// For some reason we can't use static_assert here even though this shouldn't 
	// be used used anywhere
	assert(false, "EnvSettings should never be serialised");
	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>( vl::cluster::ByteStream& msg, vl::config::EnvSettingsRefPtr &env )
{
	// For some reason we can't use static_assert here even though this shouldn't 
	// be used used anywhere
	assert(false, "EnvSettings should never be deserialised");
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
	msg << window.name << window.type << window.get_channels() << window.area 
		<< window.stereo_type << window.vert_sync << window.fsaa  << window.renderer;

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
	msg >> window.name >> window.type >> window.get_channels() >> window.area
		>> window.stereo_type >> window.vert_sync >> window.fsaa >> window.renderer;

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
	msg << chan.name << chan.wall << chan.area << chan.background_colour 
		<< chan.user_projection_left << chan.user_projection_right;

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>( vl::cluster::ByteStream& msg, vl::config::Channel &chan )
{
	msg >> chan.name >> chan.wall >> chan.area >> chan.background_colour
		>> chan.user_projection_left >> chan.user_projection_right;

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
		<< projection.horizontal;

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>(vl::cluster::ByteStream &msg, vl::config::Projection &projection)
{
	msg >> projection.type >> projection.perspective_type >> projection.fov
		>> projection.horizontal;

	return msg;
}



template<>
vl::cluster::ByteStream &
vl::cluster::operator<<( vl::cluster::ByteStream& msg, const vl::Settings& settings )
{
	// For some reason we can't use static_assert here even though this shouldn't 
	// be used used anywhere
	assert(false, "Settings should never be deserialised");
	return msg;
}

/// @todo the only information needed from these is paths, so we might as well
/// just send them without serializing the projects.
template<>
vl::cluster::ByteStream &
vl::cluster::operator>>( vl::cluster::ByteStream& msg, vl::Settings& settings )
{
	// For some reason we can't use static_assert here even though this shouldn't 
	// be used used anywhere
	assert(false, "Settings should never be deserialised");
	return msg;
}


template<>
vl::cluster::ByteStream &
vl::cluster::operator<<( vl::cluster::ByteStream& msg, vl::config::ProjSettings const &proj )
{
	// For some reason we can't use static_assert here even though this shouldn't 
	// be used used anywhere
	assert(false, "ProjSettings should never be deserialised");
	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>( vl::cluster::ByteStream& msg, vl::config::ProjSettings &proj )
{
	// For some reason we can't use static_assert here even though this shouldn't 
	// be used used anywhere
	assert(false, "ProjSettings should never be deserialised");
	return msg;
}
