/*	Joonatan Kuosa
 *	2010-02
 *
 *	Basic quaternion math class,
 *	Provides template which can be specialized based on floating point type
 *	(should work for integers or complexes also, but not quarantied).
 *
 * 	Quaternions can represent both rotation and scale, but they are
 * 	specifically designed for rotations and mixing scale and rotation
 * 	might not be the best idea anyway, but it's up to the application
 * 	programmer and not enforced by this class.
 *
 *	TODO need to run unit tests on this.
 *
 *	TODO add matrix conversions (no hurry with this one).
 */

#ifndef VL_MATH_QUATERNION_HPP
#define VL_MATH_QUATERNION_HPP

#include "vector.hpp"

namespace vl
{

namespace math
{

	// TODO implement
	template< typename float_t >
	class quaternion
	{
		public :
			quaternion( float_t w1 = 1, float_t x1 = 0, float_t y1 = 0,
					float_t z1 = 0 )
				: w(w1), x(x1), y(y1), z(z1)
			{}

			quaternion( float_t angle, vector<3, float_t> const &v )
			{
				set( angle, v );
			}

			// Construct quaternion from vector, usefull for both reflections
			// and also if you are using quaternions for translation
			// (makes the math a bit easier and consise).
			// Also in expectation of moving to dual quaternions later.
			quaternion( vector<3, float_t> const &v )
				: w(0), x(v[0]), y(v[1]), z(v[2])
			{}

			// Same as above but use individual element,
			// using references because the float_t could be complex type
			// and if it's not we optimize for doubles and long doubles anyway.
			/*
			quaternion( float_t const &vx, float_t const &vy,
					float_t const &vz )
				: w(0), x(vx), y(vy), z(vz)
			{}
			*/

			// No rotation
			void reset( )
			{
				w = 1;
				x = 0;
				y = 0;
				z = 0;
			}

			// Set from angle-axis presentation, angle is radians
			void set( float_t angle, vector<3, float_t> const &v )
			{
				w = std::cos( angle/2 );

				float_t scale = v.norm();
				x = v[0]*std::sin( angle/2 )/scale;
				y = v[1]*std::sin( angle/2 )/scale;
				z = v[2]*std::sin( angle/2 )/scale;
			}

			// Get angle-axis presentation (crude C style), angle is radians
			void get( float_t &angle, vector<3, float_t> &v ) const
			{
				angle = 2*std::acos(w);
				v[0] = x/std::sin(angle/2);
				v[1] = y/std::sin(angle/2);
				v[2] = z/std::sin(angle/2);
			}

			// Set from euler angles, all angles are radians
			// TODO add separate class to represent euler angles,
			// will replace the ambiquety between them and quaternion elems
			void set( float_t x_angle, float_t y_angle, float_t z_angle )
			{
				float_t cx = std::cos( x_angle/2 );
				float_t cy = std::cos( y_angle/2 );
				float_t cz = std::cos( z_angle/2 );

				float_t sx = std::sin( x_angle/2 );
				float_t sy = std::sin( y_angle/2 );
				float_t sz = std::sin( z_angle/2 );

				w = cx*cy*cz - sx*sy*sz;
				x = cy*cz*sx + sy*sz*cx;
				y = sy*cz*cx + cy*sz*sx;
				z = cy*sz*cx - sy*cz*sx;
			}

			// Get euler angles (crude C style for now), all angles are radians
			void get( float_t &x_angle, float_t &y_angle, float_t &z_angle )
				const
			{
				x_angle = std::atan2( 2*(w*x+y*z), 1-2*(x*x+y*y) );
				y_angle = std::asin( 2*(w*y - z*x) );
				z_angle = std::atan2( 2*(w*z+x*y), 1-2*(y*y+z*z) );
			}

			// Operators
			// All of these modify the original quaternion and return
			// reference to it so they are both memory efficent and
			// easy to compine.
			// Use the non-member functions if you don't want to modify the
			// original quaternion but want a new one instead.
			//
			// Addition and substraction (less useful really for rotations)
			quaternion<float_t> &operator+=( quaternion<float_t> const& q )
			{
				w += q.w;
				x += q.x;
				y += q.y;
				z += q.z;

				return *this;
			}

			quaternion<float_t> &operator-=( quaternion<float_t> const& q )
			{
				w -= q.w;
				x -= q.x;
				y -= q.y;
				z -= q.z;

				return *this;
			}

			// Multiplication
			// Used for compining rotations. Mind you this is not commutative
			// operation.
			quaternion<float_t> &operator*=( quaternion<float_t> const& q )
			{
				float_t w1 = w*q.w - x*q.x - y*q.y - z*q.z;
				float_t x1 = w*q.x + x*q.w + y*q.z - z*q.y;
				float_t y1 = w*q.y + y*q.w + z*q.x - x*q.z;
				float_t z1 = w*q.z + z*q.w + x*q.y - y*q.x;
				w = w1; x = x1; y = y1; z = z1;

				return *this;
			}

			// Scalar operations
			// Less usefull, but if you want to use uniform scaling also.
			quaternion<float_t> &operator*=( float_t const& s )
			{
				w *= s;
				x *= s;
				y *= s;
				z *= s;

				return *this;
			}

			quaternion<float_t> &operator/=( float_t const& s )
			{
				if( s != 0 )
				{
					w /= s;
					x /= s;
					y /= s;
					z /= s;
				}

				return *this;
			}

			// Make this quaternion to it's conjugate.
			// You probably want to use the non-member function.
			quaternion<float_t> &conj( void )
			{
				x *= -1;
				y *= -1;
				z *= -1;

				return *this;
			}

			// Normalize this quaternion. If you are using only rotations this
			// function will remove scaling.
			quaternion<float_t> &normalize( void )
			{
				if( !unit() )
				{
					float_t scale = norm();
					*this /= scale;
					/*
					x /= scale;
					y /= scale;
					z /= scale;
					w /= scale;
					*/
				}

				return *this;
			}
			// ENDOF Operations

			// Length of the quaternion, equals the uniform scaling.
			float_t norm( void ) const
			{
				return std::sqrt( x*x + y*y + z*z + w*w );
			}

			// Boolean operators
			// Is this unit quaternion.
			bool unit( void ) const
			{
				return ( norm()+EPSILON > 1 && norm()-EPSILON < 1 );
			}

			// Is this identity quaternion, that is this does not affect
			// transformation anyway what so ever.
			bool identity( void ) const
			{
				return ( unit() && w+EPSILON > 1 && w-EPSILON < 1 );
			}

			// Is this reflection quaternion.
			bool reflection( void ) const
			{
				return ( unit() && w+EPSILON > 0 && w-EPSILON < 0 );
			}
			// ENDOF Boolean operators

			// Operations on vectors or other quats.
			// Will not modify this quaternion anyway.
			//
			// Rotate 3d vector with this quaternion, if quaternion has
			// scaling we discard that scaling for this operation
			// eq. we need to make an extra copy without the scaling and
			// normalize the quaternion.
			vector<3, float_t> rotate( vector<3, float_t> const &v ) const
			{
				// For now we normalize all quaternions... easier.
				quaternion<float_t> q = *this;
				q.normalize();
				quaternion<float_t> con = q;
				q.conj();

				quaternion in(v);
				quaternion out = q*in*con;
				float_t array[3];
				array[0] = out.x;
				array[1] = out.y;
				array[2] = out.z;
				return vector<3, float>( array );
			}

			// Easy to use method which discards the angle part of the
			// quaternion, normalizes it and returns reflected vector.
			vector<3, float_t> reflect( vector<3, float_t> const &v ) const
			{
				quaternion<float_t> q = *this;
				q.w = 0;
				q.normalize();
				//quaternion<float_t> con = q;
				//q.conj();

				quaternion in(v);
				quaternion out = q*in*q;
				float_t array[3];
				array[0] = out.x;
				array[1] = out.y;
				array[2] = out.z;
				return vector<3, float>( array );
			}

			quaternion<float_t> rotate( quaternion<float_t> const &q ) const
			{
				quaternion<float_t> r = *this;
				r.normalize();
				quaternion<float_t> con = r;
				r.conj();

				return r*q*con;
			}

			quaternion<float_t> reflect( quaternion<float_t> const &q ) const
			{
				quaternion<float_t> r = *this;
				r.w = 0;
				r.normalize();

				return r*q*r;
			}

			//matrix<3, 3, float_t> toRotMatrix( void );

			//matrix<4, 4, float_t> toMatrix( void );

			// For now these are public
			float_t w;
			float_t x;
			float_t y;
			float_t z;

	};	// class quaternion

	// Arithmetic operators which makes copies
	template< typename float_t >
	inline quaternion<float_t>
	operator+( quaternion<float_t> const &q1, quaternion<float_t> const &q2 )
	{
		quaternion<float_t> r1 = q1;
		return r1 += q2;
	}

	template< typename float_t >
	inline quaternion<float_t>
	operator-( quaternion<float_t> const &q1, quaternion<float_t> const &q2 )
	{
		quaternion<float_t> r1 = q1;
		return r1 -= q2;
	}

	template< typename float_t >
	inline quaternion<float_t>
	operator*( quaternion<float_t> const &q1, quaternion<float_t> const &q2 )
	{
		quaternion<float_t> r1 = q1;
		return r1 *= q2;
	}

	template< typename float_t >
	inline quaternion<float_t>
	operator*( quaternion<float_t> const &q1, float_t const &s )
	{
		quaternion<float_t> r1 = q1;
		return r1 *= s;
	}

	template< typename float_t >
	inline quaternion<float_t>
	operator/( quaternion<float_t> const &q1, float_t const &s )
	{
		quaternion<float_t> r1 = q1;
		return r1 /= s;
	}

	/*
	template< typename float_t >
	inline quaternion<float_t>
	operator*( quaternion<float_t> const &q, vector<3, float_t> const &v )
	{
		quaternion<float_t> out;
	}

	template< typename float_t >
	inline quaternion<float_t>
	operator*( vector<3, float_t> const &v, quaternion<float_t> const &q )
	{
	}
	*/
	// ENDOF Arithmetic operators
	// ENDOF Arithmetic operators

	template< typename float_t >
	inline quaternion<float_t> normalize( quaternion<float_t> const &q )
	{
		quaternion<float_t> r = q;
		return r.normalize();
	}

	// Alternative syntax for norm
	template< typename float_t >
	inline float_t norm( quaternion<float_t> const &q )
	{
		return q.norm();
	}

	template< typename float_t >
	inline quaternion<float_t> conj( quaternion<float_t> const &q )
	{
		quaternion<float_t> r = q;
		return r.conj();
	}

	// Logical operators
	template< typename float_t >
	inline bool
	operator==( quaternion<float_t> const &q1, quaternion<float_t> const &q2 )
	{
		return ( (q1.w-EPSILON < q2.w && q1.w+EPSILON > q2.w)
				&& (q1.x-EPSILON < q2.x && q1.x+EPSILON > q2.x)
				&& (q1.y-EPSILON < q2.y && q1.y+EPSILON > q2.y)
				&& (q1.z-EPSILON < q2.z && q1.z+EPSILON > q2.z) );
	}

	template< typename float_t >
	inline bool
	operator!=( quaternion<float_t> const &q1, quaternion<float_t> const &q2 )
	{
		return !(q1 == q2);
	}
	// ENDOF Logical operators

	// Output operator, mostly for debug printing
	template< typename float_t >
	inline 
	std::ostream &operator<<( std::ostream &os,
			quaternion<float_t> const &quat )
	{
		os << quat.w << " " << quat.x << " " << quat.y << " " << quat.z << " ";
		return os;
	}

}	// namespace math

}	// namespace vl

#endif
