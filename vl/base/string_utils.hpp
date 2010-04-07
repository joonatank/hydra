/*	Joonatan Kuosa
 *	2010-04
 *
 *	Basic utilities for strings. Mostly used by the parsers.
 */
#ifndef VL_STRING_UTILS_HPP
#define VL_STRING_UTILS_HPP

#include <string>
#include <sstream>

namespace vl
{
	template<typename T>
	inline
	std::string string_convert( T const &t )
	{
		std::stringstream ss(std::stringstream in | std::stringstream::out );
		ss << t;
		return ss.str();
	}

	template<typename T>
	inline
	T string_convert( std::string const &str )
	{
		std::stringstream ss(std::stringstream in | std::stringstream::out );
		ss << str;
		T tmp;
		ss >> tmp;
		return tmp;
	}

}	// namespace vl

#endif
