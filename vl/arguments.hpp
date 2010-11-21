#ifndef VL_ARGUMENTS_HPP
#define VL_ARGUMENTS_HPP

#include <cstring>
#include <string>
#include <iostream>

namespace vl
{

struct Arguments
{
	Arguments( int const argc, char **argv )
	{
		// First is program name
		// We need at least two arguments
		// (one for the option switch and other for the real option)

		int i = 1;
		while( i < argc )
		{
			if( isOption( argv[i] ) )
			{
				if( i+1 < argc )
				{
					process( argv[i], argv[i+1] );
					i += 2;
				}
				else
				{ break; }
			}
			else
			{
				std::cerr << argv[i] << " is not an option : Not processed."
					<< std::endl;
				i++;
			}
		}
	}

	void process( char const *str1, char const *str2 )
	{
		if( isEnv(str1) )
		{
			env_path = str2;
		}
		else if( isProj(str1) )
		{
			proj_path = str2;
		}
		else if( isGlobal(str1) )
		{
			global_path = str2;
		}
		else if( isCase(str1) )
		{
			case_name = str2;
		}
		else
		{
			std::cerr << "Argument switch " << str1 << " not valid.";
		}
	}

	/// Options are always form of -X where X is an option switch
	bool isOption( char const *str ) const
	{ return( ::strncmp( str, "-", 1 ) == 0 && ::strlen( str ) == 2 ); }

	bool isEnv( char const *str ) const
	{ return( ::strcmp( str, "-e" ) == 0 ); }

	bool isProj( char const *str ) const
	{ return( ::strcmp( str, "-p" ) == 0 ); }

	bool isGlobal( char const *str ) const
	{ return( ::strcmp( str, "-g" ) == 0 ); }

	bool isCase( char const *str ) const
	{ return( ::strcmp( str, "-n" ) == 0 ); }

	std::string env_path;
	std::string proj_path;
	std::string global_path;
	std::string case_name;

};	// class Arguments

}	// namespace vl

#endif	// VL_ARGUMENTS
