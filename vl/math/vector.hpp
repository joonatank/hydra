/*	Joonatan Kuosa
 *	2010-02
 *
 *	Basic vector math class,
 *	Provides template which can be specialized based on floating point type
 *	(should work for integers or complexes also, but not quarantied).
 *	And also based on dimension.
 *
 *	TODO need to run unit tests on this.
 */

#ifndef VL_MATH_VECTOR_HPP
#define VL_MATH_VECTOR_HPP

#include <cmath>	// for std::sqrt
#include <cstring>	// for size_t
#include <iostream>	// for std::ostream

namespace vl
{
namespace math
{
	const double EPSILON = 1e-10;

	template< size_t D, typename float_t >
	class vector
	{
		public :
			vector( float_t *arr = 0, size_t size = D )
			{
				// Copy the array
				if( arr )
				{
					for( size_t i = 0; i < size; i++ )
					{ array[i] = arr[i]; }
				}
				// Initialise the array to zero
				else
				{
					for( size_t i = 0; i < size; i++ )
					{ array[i] = 0; }
				}
			}

			float_t &operator[](size_t index)
			{ return array[index]; }

			float_t const &operator[](size_t index) const
			{ return array[index]; }

			vector<D, float_t> &operator+=( vector<D, float_t> const &v )
			{
				for( size_t i = 0; i < D; i++ )
				{ array[i] += v[i]; }
				return *this;
			}

			vector<D, float_t> &operator-=( vector<D, float_t> const &v )
			{
				for( size_t i = 0; i < D; i++ )
				{ array[i] -= v[i]; }
				return *this;
			}

			vector<D, float_t> &operator*=( vector<D, float_t> const &v )
			{
				for( size_t i = 0; i < D; i++ )
				{ array[i] *= v[i]; }
				return *this;
			}

			vector<D, float_t> &operator*=( float_t const &s )
			{
				for( size_t i = 0; i < D; i++ )
				{ array[i] *= s; }
				return *this;
			}

			vector<D, float_t> &operator/=( float_t const &s )
			{
				for( size_t i = 0; i < D; i++ )
				{ array[i] /= s; }
				return *this;
			}

			// Bit special vector scalar operators, allows to add or substract
			// specific scalar from every element in the vector
			vector<D, float_t> &operator+=( float_t const &s )
			{
				for( size_t i = 0; i < D; i++ )
				{ array[i] += s; }
				return *this;
			}

			vector<D, float_t> &operator-=( float_t const &s )
			{
				for( size_t i = 0; i < D; i++ )
				{ array[i] -= s; }
				return *this;
			}

			vector<D, float_t> &normalize( void )
			{
				// Normalize only if not already unit vector
				float_t scale = norm(); 
				if( !unit() )
				{
					for( size_t i = 0; i < D; i++ )
					{ array[i] /= scale; }
				}

				return *this;
			}

			float_t norm( void ) const
			{
				return std::sqrt( dot( *this ) );
			}

			bool unit( void ) const
			{
				return ( norm()+EPSILON > 1 && norm()-EPSILON < 1 );
			}

			bool zero( void ) const
			{
				for( size_t i; i < D; i++ )
				{ 
					if( array[i] + EPSILON < 0 || array[i] - EPSILON > 0)
					{ return false; }
				}

				return true;
			}

			float_t dot( vector<D, float_t> v ) const
			{
				float_t ret = 0;
				for( size_t i = 0; i < D; i++ )
				{ ret += array[i]*v.array[i]; }

				return ret;
			}

		private :
			float_t array[D];

	};	// class vector<D, float_t>

	template< size_t D, typename float_t >
	inline float_t
	dot( vector<D, float_t> const &v1, vector<D, float_t> const &v2 )
	{
		return v1.dot( v2 );
	}

	// TODO need to check the base vectors for this
	template< typename float_t >
	inline vector<3, float_t>
	cross( vector<3, float_t> const &v1, vector<3, float_t> const &v2 )
	{
		float_t v[3];

		v[0] = v1[1]*v2[2] - v1[2]*v2[1];
		v[1] = v1[2]*v2[0] - v1[0]*v2[2];
		v[2] = v1[0]*v2[1] - v1[1]*v2[0];

		return vector<3, float_t>( v );
	}

	// specialization for 3d vectors
	// TODO this should really be implemented using inheritance, we only need
	// to provide some niche interface for this one.
	/*
	template< typename float_t >
	class vector< 3, float_t >
	{
		public :
			vector( float_t x1 = 0, float_t y1 = 0, float_t z1 = 0 )
				: array[0](x1), array[1](y1), array[2](z1)
			{}

			float_t &x( void )
			{ return array[0]; }

			float_t &y( void )
			{ return array[1]; }

			float_t &z( void )
			{ return array[2]; }

			float_t &operator[](size_t index)
			{ return array[index]; }

			vector<3, float_t> &operator+=( vector<3, float_t> const &v )
			{
				for( size_t i = 0; i < 3; i++ )
				{ array[i] += v.array[i]; }
				return *this;
			}

			vector<3, float_t> &operator-=( vector<3, float_t> const &v )
			{
				for( size_t i = 0; i < 3; i++ )
				{ array[i] -= v.array[i]; }
				return *this;
			}

			vector<3, float_t> &operator*=( vector<3, float_t> const &v )
			{
				for( size_t i = 0; i < 3; i++ )
				{ array[i] *= v.array[i]; }
				return *this;
			}

			vector<3, float_t> &operator*=( float_t const &s )
			{
				for( size_t i = 0; i < 3; i++ )
				{ array[i] *= s; }
				return *this;
			}

		private :
			float_t array[3];
		
	};	// class vector<3, float_t>
	*/

	typedef vector<3, double> vec3d;

	// Arithmetic operators
	template< size_t D, typename float_t >
	inline vector<D, float_t>
	operator+( vector<D, float_t> const &v1, vector<D, float_t> const &v2 )
	{
		vector<D, float_t> ret = v1;
		return (ret += v2);
	}

	template< size_t D, typename float_t >
	inline vector<D, float_t>
	operator-( vector<D, float_t> const &v1, vector<D, float_t> const &v2 )
	{
		vector<D, float_t> ret = v1;
		return (ret -= v2);
	}

	template< size_t D, typename float_t >
	inline vector<D, float_t>
	operator*( vector<D, float_t> const &v1, vector<D, float_t> const &v2 )
	{
		vector<D, float_t> ret = v1;
		return (ret *= v2);
	}

	template< size_t D, typename float_t >
	inline vector<D, float_t>
	operator*( vector<D, float_t> const &v, float_t const &s )
	{
		vector<D, float_t> ret = v;
		return (ret *= s);
	}

	template< size_t D, typename float_t >
	inline vector<D, float_t>
	operator/( vector<D, float_t> const &v, float_t const &s )
	{
		vector<D, float_t> ret = v;
		return (ret /= s);
	}
	// ENDOF Arithmetic operators
	
	// Logic operators
	template< size_t D, typename float_t >
	inline bool
	operator==( vector<D, float_t> const &v1, vector<D, float_t> const &v2 )
	{
		for( size_t i = 0; i < D; i++ )
		{
			if( v1[i]-EPSILON > v2[i] || v1[i]+EPSILON < v2[i] )
			{ return false; }
		}
		return true;
	}

	template< size_t D, typename float_t >
	inline bool
	operator!=( vector<D, float_t> const &v1, vector<D, float_t> const &v2 )
	{
		return !( v1 == v2 );
	}
	// ENDOF Logic operators


	// Text output
	template< size_t D, typename float_t >
	inline std::ostream &
	operator<<( std::ostream &os, vector<D, float_t> const &vec )
	{
		for( size_t i = 0; i < D; i++ )
		{ os << vec[i] << " "; }

		return os;
	}

}	// namespace math

}	// namespace vl

#endif
