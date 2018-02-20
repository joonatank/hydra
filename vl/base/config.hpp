/**
 *	Copyright (c) 2011 - 2014 Savant Simulators
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
#include <iostream>

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
	ST_DEFAULT,
	ST_QUAD_BUFFER,
	ST_SIDE_BY_SIDE,
	ST_TOP_BOTTOM,
	ST_OCULUS,
};

/// Duplicating stereo and Window configs for Oculus
/// More locigal for the user, stereo config is copied from Window
enum WindowType
{
	WT_WINDOW,
	WT_OCULUS,
};

std::string convert_stereo(StereoType const &stereo);

StereoType convert_stereo(std::string const &stereo);

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

};	// struct Tracking

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
		USER,
	};

	Projection(void)
		: type(PERSPECTIVE)
		, perspective_type(FOV)
		, fov(60)
		, horizontal(-1)
		, use_asymmetric_stereo(false)
		, hmd(false)
	{}

	bool empty(void) const
	{
		if(type == PERSPECTIVE && fov == 60 && perspective_type == FOV 
			&& !use_asymmetric_stereo && wall.empty())
		{ return true; }
	}

	Type type;

	PerspectiveType perspective_type;

	Wall wall;

	// Field-of-View in Degrees
	double fov;

	// Horizontal in percentage zero is floor and one is ceiling
	double horizontal;

	bool use_asymmetric_stereo;

	bool hmd;

};	// struct Projection

std::ostream &operator<<(std::ostream &os, Projection const &p);

/// @todo Channel should have projection parameter
/// At the moment we need to use wall if we want to set projection
struct HYDRA_API Channel
{
	Channel(std::string const &nam, Rect<double> const &a, 
		Projection const &p, vl::Colour const &colour)
		: name(nam), projection(p), area(a), texture_size(0, 0), fsaa(1),
		background_colour(colour)
	{}

	/// Minimal constructor
	Channel(std::string const &nam)
		: name(nam)
		, area(1, 1, 0, 0)
		, texture_size(0, 0)
		, fsaa(1)
		, background_colour(0, 0, 0)
	{}

	/// Default constructor for vector resizes
	Channel( void )
	{}

	bool empty( void ) const
	{ return( name.empty() && projection.empty() && area == Rect<double>() ); }

	std::string name;

	Projection projection;

	// Size of the Channel a percentage from the Window
	Rect<double> area;

	// Size of the texture if using FBO rendering, defaults to real size of the Channel
	vl::vec2i texture_size;

	// anti aliasing
	uint32_t fsaa;

	vl::Colour background_colour;

	// User projection matrices that can be used instead of wall or fov
	// @todo not sure if this is proper place for them though.
	Ogre::Matrix4 user_projection_left;
	Ogre::Matrix4 user_projection_right;

};	// struct Channel

std::ostream &operator<<(std::ostream &os, Channel const &c);

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
		: type(FBO)
		, hardware_gamma(false)
		, stereo_type(ST_DEFAULT)
		, ipd(0.065)
		, fps(60)
	{}

	Type type;

	bool hardware_gamma;

	// Inter pupilar distance
	double ipd;
	uint32_t fps;

	// Stereo type
	// Provides a global default for all Windows
	// Window overrides the default value here.
	// Having window specific values allows us to mix and match
	// stereo types, windows with stereo and without, HMDs and projectors.
	StereoType stereo_type;

};	// struct Renderer

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
		return( name.empty() && _channels.empty() && area.empty() );
	}

	// Name of the window
	std::string name;
	// What kind of window we have
	WindowType type;
	// @todo why is Renderer in Window and not the otherway around?
	Renderer renderer;

	// Window size
	Rect<int> area;
	// Extra parameters to pass to window
	NamedParamList params;
	// What kind of stereo if any we are using
	StereoType stereo_type;
	// Not tested that much, but should be working
	bool vert_sync;
	// Fullscreen antialising
	// @todo should this be window or Renderer specific?
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
	{}

	// Default constructor to allow vector resize
	Node( void )
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

};	// struct Node

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

};	// struct Server

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

};	// struct Program

}	// namespace config

}	// namespace vl

#endif // HYDRA_BASE_CONFIG_HPP
