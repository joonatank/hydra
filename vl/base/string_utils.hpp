/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-04
 *	@file base/string_utils.hpp
 *
 *	This file is part of Hydra VR game engine.
 *
 *	Basic utilities for strings. Mostly used by the parsers.
 *
 *	Update 2011-01
 *	Added to_lower and to_upper. Change case of the whole string.
 */
#ifndef HYDRA_STRING_UTILS_HPP
#define HYDRA_STRING_UTILS_HPP

/// c++ standard headers
// Necessary for std::find
#include <algorithm>
// Necessary for vector parameter
#include <vector>
// Necessary because these operate on std::strings
#include <string>
// Necessary for conversions
#include <sstream>

// Necessary for throwing
#include "exceptions.hpp"

namespace vl
{
	extern const char* WHITESPACE;

	inline bool has_only_whitespace(std::string const str)
	{
		if(str.find_first_not_of(WHITESPACE) == std::string::npos )
		{ return true; }

		return false;
	}

	/// @brief convert strings to contain only lower case characters
	/// @param str string to convert
	/// @throws nothing
	inline
	void to_lower( std::string &str )
	{
		for( size_t i = 0; i < str.length(); ++i )
		{
			str.at(i) = ::tolower(str.at(i));
		}
	}

	/// @brief convert strings to contain only upper case characters
	/// @param str string to convert
	/// @throws nothing
	inline
	void to_upper( std::string &str )
	{
		for( size_t i = 0; i < str.length(); ++i )
		{
			str.at(i) = ::toupper(str.at(i));
		}
	}

	/// Breaks a string containing multiple substrings separated by a delimiter
	/// to those substrings and adds them to the broken_path vector parameter
	void break_string_down( std::vector<std::string> &broken_path,
							std::string &longString,
							char delimeter );

	/// @brief replaces Windows line endings with unix ones
	/// @param str string which should have it's line endings altered
	/// @throws nothing
	void replace_line_endings(std::string &str);

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

	template<>
	inline
	bool from_string( std::string const &str )
	{
		std::string tmp = str;
		vl::to_lower(tmp);	
		if( tmp == "on" || tmp == "true" || tmp == "1" )
		{
			return true;
		}

		return false;
	}

	std::string generate_random_string(size_t len = 8);

}	// namespace vl

#endif	// HYDRA_STRING_UTILS_HPP
