/**
 *	Copyright (c) 2010 - 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-12
 *	@file base/string_utils.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

/**	Basic utilities for strings. Mostly used by the parsers.
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

// Necessary for exposing the API
#include "defines.hpp"

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

	inline std::string to_lower(std::string const &str)
	{
		std::string tmp(str);
		to_lower(tmp);
		return tmp;
	}
	
	inline std::string to_upper(std::string const &str)
	{
		std::string tmp(str);
		to_upper(tmp);
		return tmp;
	}

	/// Breaks a string containing multiple substrings separated by a delimiter
	/// to those substrings and adds them to the broken_path vector parameter
	HYDRA_API void break_string_down( std::vector<std::string> &broken_path,
							std::string &longString,
							char delimeter );

	HYDRA_API void break_string_down( std::vector<std::string> &parts,
							std::string const &longString,
							char delimeter );

	/// @brief replaces Windows line endings with unix ones
	/// @param str string which should have it's line endings altered
	/// @throws nothing
	HYDRA_API void replace_line_endings(std::string &str);

	template<typename T>
	inline
	std::string to_string( T const &t )
	{
		std::stringstream ss(std::stringstream::in | std::stringstream::out );
		ss << t;
		return ss.str();
	}


	template<>
	inline
	std::string to_string(bool const &val)
	{
		if(val)
		{ return "true"; }
		else
		{ return "false"; }
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

	HYDRA_API std::string generate_random_string(size_t len = 8);

}	// namespace vl

#endif	// HYDRA_STRING_UTILS_HPP
