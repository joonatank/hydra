/**
 *	Copyright (c) 2011 - 2013 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2013-07
 *	@file math/colour.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

#ifndef HYDRA_MATH_COLOUR_HPP
#define HYDRA_MATH_COLOUR_HPP

#include "math/types.hpp"

namespace vl
{

class Colour
{
public :

	Colour(vl::scalar r_, vl::scalar g_, vl::scalar b_, vl::scalar a_ = 1.0)
		: r(r_), g(g_), b(b_), a(a_)
	{}

	Colour(int r_, int g_, int b_, int a_ = 255)
	{
		r = ((vl::scalar)r_)/255.0;
		g = ((vl::scalar)g_)/255.0;
		b = ((vl::scalar)b_)/255.0;
		a = ((vl::scalar)a_)/255.0;
	}

	Colour(void)
		: r(0), g(0), b(0), a(1.0)
	{}

	vl::scalar r;
	vl::scalar g;
	vl::scalar b;
	vl::scalar a;

};	 // class Colour

inline
std::ostream &operator<<(std::ostream &os, Colour const &c)
{
	os << "(" << c.r << ", " << c.g << ", " << c.b << ", " << c.a << ")";
	return os;
}

}	// namespace vl

#endif // HYDRA_MATH_COLOUR_HPP
