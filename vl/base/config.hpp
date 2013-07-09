/**
 *	Copyright (c) 2011 - 2013 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2013-07
 *	@file config.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

#ifndef HYDRA_BASE_CONFIG_HPP
#define HYDRA_BASE_CONFIG_HPP

#include "rect.hpp"
#include "wall.hpp"
// Necessary for Channel background colour
#include "math/colour.hpp"
// Necessary for HYDRA_API
#include "defines.hpp"
// Necessary for NamedParamList
// @todo this is a bit of an overkill to include though
#include "typedefs.hpp"

#include <string>
#include <stdint.h>

namespace vl
{

namespace config
{

// Three value configs
enum CFG
{
	// Uses parents settings, might be global config settings or program defaults
	CFG_DEFAULT,
	CFG_OFF,
	CFG_ON,
};

enum LogLevel
{
	LL_LOW = 0,
	LL_NORMAL = 1,
	LL_BOREME = 2,
};

enum StereoType
{
	ST_OFF,
	ST_QUAD_BUFFER,
	ST_SIDE_BY_SIDE,
	ST_TOP_BOTTOM,
};

struct Tracking
{
	Tracking( std::string const &file_name, bool u = "true" )
		: file(file_name), use(u)
	{}

	bool operator==( Tracking const &other ) const
	{
		if( file == other.file )
		{ return true; }

		return false;
	}

	std::string file;
	bool use;
};

/// @todo Channel should have projection parameter
/// At the moment we need to use wall if we want to set projection
struct HYDRA_API Channel
{
	Channel(std::string const &nam, Rect<double> const &a, 
		Wall const &w, vl::Colour const &colour)
		: name(nam), wall(w), area(a), background_colour(colour)
	{}

	/// Minimal constructor
	Channel(std::string const &nam)
		: name(nam)
		, area(1, 1, 0, 0)
		, background_colour(0, 0, 0)
	{}

	/// Default constructor for vector resizes
	Channel( void )
	{}

	bool empty( void ) const
	{ return( name.empty() && wall.empty() && area == Rect<double>() ); }

	std::string name;

	Wall wall;

	Rect<double> area;

	vl::Colour background_colour;

};	// struct Channel

struct HYDRA_API Projection
{
	enum Type
	{
		PERSPECTIVE,
		ORTHO,
	};

	enum PerspectiveType
	{
		WALL,
		FOV,
	};

	Projection(void)
		: type(PERSPECTIVE)
		, perspective_type(FOV)
		, fov(60)
		, horizontal(-1)
		, head_x(false)
		, head_y(true)
		, head_z(false)
		, use_asymmetric_stereo(false)
	{}

	Type type;

	PerspectiveType perspective_type;

	// Field-of-View in Degrees
	double fov;

	// Horizontal in percentage zero is floor and one is ceiling
	double horizontal;

	bool head_x;
	bool head_y;
	bool head_z;

	bool modify_transformations;

	bool use_asymmetric_stereo;
};

/// The rendering element that can be individually rendered
/// usually either a window or a FBO
/// this is completely independent of the two though and does not reference 
/// either of them.
struct HYDRA_API Renderer
{
	enum Type
	{
		WINDOW,
		FBO,
		DEFERRED,
	};

	Renderer(void)
		: type(WINDOW)
		, projection()
		, hardware_gamma(false)
	{}

	Type type;
	Projection projection;

	bool hardware_gamma;
};

struct HYDRA_API Window
{
	Window( std::string const &nam, int width, int height, int px, int py,
			StereoType stereo_t = ST_OFF );

	Window( std::string const &nam, Rect<int> area, StereoType stereo_t = ST_OFF );

	// Default constructor to allow vector resize
	Window(void)
	{
		clear();
	}

	void clear(void);

	// Wether or not the Window has been initialised
	bool empty( void ) const
	{
		return( name.empty() && _channels.empty() && rect.empty() );
	}

	// Name of the window
	std::string name;

	Renderer renderer;

	Rect<int> rect;

	NamedParamList params;

	StereoType stereo_type;

	bool nv_swap_sync;
	uint32_t nv_swap_group;
	uint32_t nv_swap_barrier;

	bool vert_sync;
	bool input_handler;
	int n_display;

	int fsaa;

	void add_channel(Channel const &channel);

	bool has_channel(std::string const &name) const;

	Channel const &get_channel(size_t i) const
	{ return _channels.at(i); }

	Channel &get_channel(size_t i)
	{ return _channels.at(i); }

	size_t get_n_channels(void) const
	{ return _channels.size(); }


	/// @internal distribution access
	std::vector<Channel> const &get_channels(void) const
	{ return _channels; }

	std::vector<Channel> &get_channels(void)
	{ return _channels; }

private :
	std::vector<Channel> _channels;

};	// struct Window

struct HYDRA_API Node
{
	Node( std::string const &nam )
		: name(nam)
		, gui_enabled(false)
	{}

	// Default constructor to allow vector resize
	Node( void )
		: gui_enabled(false)
	{}

	bool empty( void ) const
	{ return name.empty() && windows.empty(); }

	void addWindow( Window const &window );

	Window &getWindow(size_t i);
	
	Window const &getWindow(size_t i) const;

	size_t getNWindows( void ) const
	{ return windows.size(); }

	std::vector<Window> const &getWindows( void ) const
	{ return windows; }

	std::string name;
	std::vector<Window> windows;
	bool gui_enabled;
};

struct HYDRA_API Server
{
	Server( uint16_t por, std::string const hostnam )
		: port(por), hostname(hostnam)
	{}

	// Default constructor
	Server(void)
	{}

	uint16_t port;
	std::string hostname;
};

/// External program description
struct HYDRA_API Program
{
	std::string name;
	std::string directory;
	std::string command;
	std::vector<std::string> params;
	bool use;
	bool new_console;

	Program(void)
		: use(false)
		, new_console(false)
	{}
};

}	// namespace config

}	// namespace vl

#endif // HYDRA_BASE_CONFIG_HPP
