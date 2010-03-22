/*	Joonatan Kuosa
 *	2010-03
 *
 *	Base class for some math helper functions and typedefs for the math
 *	types we use.
 *	e.g. here you can change the vector, quaternion, matrix, scalar classes
 *	to use another library.
 */
#ifndef VL_MATH_MATH_HPP
#define VL_MATH_MATH_HPP

// For MSV compilers to get M_PI
#define _USE_MATH_DEFINES
#include <math.h>

#include <vmmlib/vector.hpp>
#include <vmmlib/math.hpp>
#include <vmmlib/quaternion.hpp>

namespace vl
{
	typedef vmml::quaterniond quaternion;
	typedef vmml::mat4d matrix;
	typedef vmml::vec3d vector;
	typedef double scalar;

	const scalar epsilon = 1e-10;
	
	inline bool equal( scalar const &a, scalar const &b )
	{
		if( a-epsilon < b && a+epsilon > b )
		{ return true; }
		return false;
	}

	inline bool equal( vector const &a, vector const &b )
	{
		if( (a.x()-epsilon < b.x() && a.x()+epsilon > b.x())
			&& (a.y()-epsilon < b.y() && a.y()+epsilon > b.y())
			&& (a.z()-epsilon < b.z() && a.z()+epsilon > b.z())
		  )
		{ return true; }
		return false;
	}

	inline bool equal( quaternion const &a, quaternion const &b )
	{
		if( (a.x()-epsilon < b.x() && a.x()+epsilon > b.x())
			&& (a.y()-epsilon < b.y() && a.y()+epsilon > b.y())
			&& (a.z()-epsilon < b.z() && a.z()+epsilon > b.z())
			&& (a.w()-epsilon < b.w() && a.w()+epsilon > b.w())
		  )
		{ return true; }
		return false;
	}

	//template< size_t N >
	//using cml::zero<N>();
};

#endif
