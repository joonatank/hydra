/**
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-10
 *	@file base/wall.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

#ifndef HYDRA_BASE_WALL_HPP
#define HYDRA_BASE_WALL_HPP

#include <vector>
#include <string>

namespace vl
{

/// @todo change vectors to normal arrays constant size of three naturally
/// they can be used to construct Ogre::Vectors easily
class Wall
{
public :
	Wall( std::string const &nam, std::vector<double> b_left,
			std::vector<double> b_right, std::vector<double> t_left )
		: name(nam), bottom_left(b_left),
			bottom_right(b_right), top_left(t_left)
	{}

	// Default constructor to allow vector resize
	Wall(void)
	{}

	// Copy constructor
	Wall(Wall const &w)
	{
		copy(w);
	}

	Wall &operator=(Wall const &w)
	{
		copy(w);
		return *this;
	}

	void copy(Wall const &w)
	{
		name = w.name;
		bottom_left = w.bottom_left;
		bottom_right = w.bottom_right;
		top_left = w.top_left;
	}

	// Whether or not the Wall has been initialised
	bool empty(void) const
	{
		return( name.empty() && bottom_left.empty()
				&& bottom_right.empty() && top_left.empty() );
	}

	// Name of the wall
	std::string name;

	// Bottom left coordinates for this wall
	std::vector<double> bottom_left;
	// Bottom right coordinates for this wall
	std::vector<double> bottom_right;
	// Top left coordinates for this wall
	std::vector<double> top_left;

};	// class Wall

}	// namespace vl

#endif	// HYDRA_BASE_WALL_HPP
