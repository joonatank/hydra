/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-03
 *	@file logger.hpp
 *
 */

#ifndef VL_LOGGER_HPP
#define VL_LOGGER_HPP

#include <streambuf>
#include <sstream>

namespace vl
{

enum LOG_TYPE
{
	LOG_NONE,
	LOG_OUT,
	LOG_ERR,
};

class Logger
{
public :
	Logger( void );

	~Logger( void );

	// TODO not used
	void setVerbose( bool v )
	{ _verbose = v; }

	bool getVerbose( void ) const
	{ return _verbose; }

	void setOutputFile( std::string const &filename )
	{ _output_filename = filename; }

	void setErrorFile( std::string const &filename )
	{ _error_filename = filename; }

	std::stringstream &cout( void )
	{ return _cout_ss; }

	std::stringstream &cerr( void )
	{ return _cerr_ss; }

	// TODO separate cout and cerr for python

	// TODO logging data to both a file and an endpoint

/// Data
private :
	bool _verbose;

	// TODO add endpoints for output
	std::streambuf *_cout_endpoint;
	std::streambuf *_cerr_endpoint;

	// Where the output is stored when we do not have endpoints
	std::stringstream _cout_ss;
	std::stringstream _cerr_ss;

	// old streambuffers so we can restore them later
	std::streambuf *_old_cout;
	std::streambuf *_old_cerr;

	std::string _output_filename;
	std::string _error_filename;

};	// namespace Logger

}	// namespace vl

#endif	// VL_LOGGER_HPP
