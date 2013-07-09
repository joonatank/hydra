/**
 *	Copyright (c) 2011 - 2013 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2013-07
 *	@file config.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

#include "config.hpp"

#include "exceptions.hpp"

vl::config::Window::Window(std::string const &nam, int width, int height, int px, int py,
		StereoType stereo_t)	
{
	clear();

	name = nam;
	rect = Rect<int>(width, height, px, py);
	stereo_type = stereo_t;

	if( rect.h < 0 || rect.w < 0 )
	{
		std::string desc("Width or height of a Window can not be negative");
		BOOST_THROW_EXCEPTION( vl::invalid_settings() << vl::desc(desc) );
	}
}

vl::config::Window::Window(std::string const &nam, Rect<int> area, StereoType stereo_t)
{
	clear();
	
	name = nam;
	rect = area;
	stereo_type = stereo_t;

	if( rect.h < 0 || rect.w < 0 )
	{
		std::string desc("Width or height of a Window can not be negative");
		BOOST_THROW_EXCEPTION( vl::invalid_settings() << vl::desc(desc) );
	}
}

void
vl::config::Window::clear(void)
{
	stereo_type = ST_OFF;
	vert_sync = false;
	input_handler = true;

	n_display = -1;

	fsaa = 0;

	nv_swap_sync = false;
	nv_swap_group = 0;
	nv_swap_barrier = 0;
}

void
vl::config::Window::add_channel(Channel const &channel)
{
	if(has_channel(channel.name))
	{ BOOST_THROW_EXCEPTION(vl::invalid_settings() << vl::desc("Can't have two channels with same name")); }

	_channels.push_back(channel);
}

bool
vl::config::Window::has_channel(std::string const &name) const
{
	for(size_t i = 0; i < _channels.size(); ++i)
	{
		if(_channels.at(i).name == name)
		{ return true; }
	}

	return false;
}
