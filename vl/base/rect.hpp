/**
 *	Copyright (c) 2011 - 2013 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2013-07
 *	@file base/rect.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

#ifndef HYDRA_BASE_RECT_HPP
#define HYDRA_BASE_RECT_HPP

// Necessary for printing
#include <iostream>

namespace vl
{

template<typename T>
struct Rect
{
	Rect(void)
	{ clear(); }

	Rect(T pw, T ph, T px, T py) : w(pw), h(ph), x(px), y(py) {}

	bool operator==(Rect const &other) const
	{
		if(other.w == w && other.h == h && other.x == x && other.y == y)
		{ return true; }
		return false;
	}

	bool valid(void) const
	{ return (w > 0 && h > 0); }

	bool empty(void) const
	{ return( w == 0 && h == 0 && x == 0 && y == 0); }

	void clear(void)
	{ w = 0; h = 0; x = 0; y = 0; }

	T w;
	T h;
	T x;
	T y;
};

template<typename T>
std::ostream &operator<<(std::ostream &os, Rect<T> const &r)
{
	os << "[" << r.w << ", " << r.h << "] [" << r.x << ", " << r.y << "]";
	return os;
}

}	// namespace vl

#endif // HYDRA_BASE_RECT_HPP
