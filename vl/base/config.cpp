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
#include "string_utils.hpp"

/// ------------------------- Global -----------------------------------------
std::string
vl::config::convert_stereo(vl::config::StereoType const &stereo)
{
	switch(stereo)
	{
	case ST_OFF :
		return std::string("off");
	case ST_DEFAULT :
		return std::string("default");
	case ST_QUAD_BUFFER :
		return std::string("quad_buffer");
	case ST_SIDE_BY_SIDE :
		return std::string("side_by_side");
	case ST_TOP_BOTTOM : 
		return std::string("top_bottom");
	case ST_OCULUS : 
		return std::string("oculus");
	}
}

vl::config::StereoType
vl::config::convert_stereo(std::string const &stereo)
{
	std::string const str = vl::to_lower(stereo);
	if(str == "off")
	{ return ST_OFF; }
	else if(str == "quad_buffer")
	{ return ST_QUAD_BUFFER; }
	else if(str == "side_by_side")
	{ return ST_SIDE_BY_SIDE; }
	else if(str == "top_bottom")
	{ return ST_TOP_BOTTOM; }
	else if(str == "oculus")
	{ return ST_OCULUS; }
	else
	{ return ST_DEFAULT; }
}

std::ostream &
vl::config::operator<<(std::ostream &os, vl::config::Channel const &c)
{
	os << c.name << " : " << c.wall << " : " << c.area 
		<< " : " << c.background_colour << std::endl;
	return os;
}

/// ------------------------- Projection -------------------------------------
std::ostream &
vl::config::operator<<(std::ostream &os, vl::config::Projection const &p)
{
	std::string type("perspective");
	if(p.type == Projection::ORTHO)
	{ type = "ortho"; }

	std::string p_type("wall");
	if(p.perspective_type == Projection::FOV)
	{ p_type = "fov"; }

	os << "Projection : " << type << " with " << p_type << " perspective"
		<< " fov " << p.fov << " and horizontal " << p.horizontal;
	if(p.use_asymmetric_stereo)
	{ os << " using asymmetric stereo."; }
	else
	{ os << " not using asymmetric stereo."; }

	return os;
}


/// ------------------------- Window -----------------------------------------
vl::config::Window::Window(std::string const &nam, int width, int height, int px, int py,
		StereoType stereo_t)	
{
	clear();

	name = nam;
	area = Rect<int>(width, height, px, py);
	stereo_type = stereo_t;

	if(area.h < 0 || area.w < 0)
	{
		std::string desc("Width or height of a Window can not be negative");
		BOOST_THROW_EXCEPTION( vl::invalid_settings() << vl::desc(desc) );
	}
}

vl::config::Window::Window(std::string const &nam, Rect<int> a, StereoType stereo_t)
{
	clear();
	
	name = nam;
	area = a;
	stereo_type = stereo_t;

	if(area.h < 0 || area.w < 0)
	{
		std::string desc("Width or height of a Window can not be negative");
		BOOST_THROW_EXCEPTION( vl::invalid_settings() << vl::desc(desc) );
	}
}

void
vl::config::Window::clear(void)
{
	name.clear();
	type = WT_WINDOW;
	params.clear();
	stereo_type = ST_OFF;
	vert_sync = false;

	fsaa = 0;
	area.clear();
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

/// ------------------------ Node --------------------------------------------
void
vl::config::Node::addWindow(vl::config::Window const &window)
{
	// Check that there is not already a Window with the same name
	std::vector<Window>::iterator iter;
	for( iter = windows.begin(); iter != windows.end(); ++iter )
	{
		if( iter->name == window.name )
		{ BOOST_THROW_EXCEPTION( vl::invalid_settings() ); }
	}

	windows.push_back(window);
}

vl::config::Window &
vl::config::Node::getWindow(size_t i)
{ return windows.at(i); }

vl::config::Window const &
vl::config::Node::getWindow( size_t i ) const
{ return windows.at(i); }
