/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-03
 *	@file logger.cpp
 *
 */

#include "logger.hpp"

#include <iostream>
#include <fstream>

vl::Logger::Logger(void )
	: _verbose(false)
	, _cout_endpoint(0)
	, _cerr_endpoint(0)
	, _cout_ss()
	, _cerr_ss()
	, _old_cout(std::cout.rdbuf())
	, _old_cerr(std::cerr.rdbuf())
	, _output_filename("temp_log_cout.txt")
	, _error_filename("temp_log_cerr.txt")
{
	std::cout.rdbuf(_cout_ss.rdbuf());
	std::cerr.rdbuf(_cerr_ss.rdbuf());
}

vl::Logger::~Logger(void )
{
	// flush the stringstreams to files
	if( !_cerr_ss.str().empty() )
	{
		std::ofstream file( _error_filename.c_str() );
		file << _cerr_ss.str();
	}
	if( !_cout_ss.str().empty() )
	{
		std::ofstream file( _output_filename.c_str() );
		file << _cout_ss.str();
	}

	// restore old output buffer
	std::cout.rdbuf(_old_cout);
	std::cerr.rdbuf(_old_cerr);
}

