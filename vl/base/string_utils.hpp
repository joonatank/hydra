/*	Joonatan Kuosa
 *	2010-04
 *
 *	Basic utilities for strings. Mostly used by the parsers.
 */
#ifndef VL_STRING_UTILS_HPP
#define VL_STRING_UTILS_HPP

/// c++ standard headers
// Necessary for std::find
#include <algorithm>
// Necessary for vector parameter
#include <vector>
// Necessary because these operate on std::strings
#include <string>
// Necessary for conversions
#include <sstream>

/// eqOgre headers
// Necessary for throwing
#include "exceptions.hpp"

namespace vl
{
	/// Breaks a string containing multiple substrings separated by a delimiter
	/// to those substrings and adds them to the broken_path vector parameter
	inline
	void break_string_down( std::vector<std::string> &broken_path,
							std::string &longString,
							char delimeter )
	{
		std::string::iterator begin = longString.begin();
		std::string::iterator path_iter
			= std::find( begin, longString.end(), delimeter );

		while( path_iter != longString.end() )
		{
			if( path_iter == begin )
			{ BOOST_THROW_EXCEPTION( vl::exception() << vl::desc( "Iterators incorrect" ) ); }

			std::string str;
			str.insert( str.begin(), begin, path_iter );
			broken_path.push_back( str );
			begin = ++path_iter;
			path_iter = std::find( begin, longString.end(), delimeter );
		}

		std::string str;
		str.insert( str.begin(), begin, path_iter );
		broken_path.push_back( str );
	}

	template<typename T>
	inline
	std::string to_string( T const &t )
	{
		std::stringstream ss(std::stringstream::in | std::stringstream::out );
		ss << t;
		return ss.str();
	}

	template<typename T>
	inline
	T from_string( std::string const &str )
	{
		std::stringstream ss(std::stringstream::in | std::stringstream::out );
		ss << str;
		T tmp;
		ss >> tmp;
		return tmp;
	}

}	// namespace vl

#endif
