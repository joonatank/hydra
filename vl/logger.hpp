/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-03
 *	@file logger.hpp
 *
 */

#ifndef VL_LOGGER_HPP
#define VL_LOGGER_HPP

#include <streambuf>
#include <sstream>

#include <iosfwd>							// streamsize
#include <boost/iostreams/categories.hpp>	// sink_tag
#include <boost/iostreams/concepts.hpp>		// sink
#include <boost/iostreams/stream.hpp>

#include <vector>

namespace io = boost::iostreams;

namespace vl
{

enum LOG_TYPE
{
	LOG_NONE,
	LOG_OUT,
	LOG_ERR,
};

struct LogMessage
{
LogMessage( LOG_TYPE ty = LOG_NONE, double tim = 0, std::string const &msg = std::string() )
	: type(ty), time(tim), message(msg)
{}

LOG_TYPE type;
double time;
std::string message;

};	// class LogMessage

inline std::ostream &
operator<<(std::ostream &os, LogMessage const &msg)
{
	os << msg.type << " " << msg.time << "s " << msg.message;
	return os;
}

class Logger;

class sink : public boost::iostreams::sink
{
public :
	sink( Logger &logger, LOG_TYPE type );

	std::streamsize write(const char* s, std::streamsize n);

	LOG_TYPE getType( void ) const
	{ return _type; }

	Logger &getLogger( void )
	{ return _logger; }

private :
	Logger &_logger;
	LOG_TYPE _type;
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

	void logMessage( LOG_TYPE type, std::string const &str );

	bool newMessages(void) const
	{ return !_messages.empty(); }

	size_t nMessages(void) const
	{ return _messages.size(); }

	LogMessage popMessage(void);

	// TODO separate cout and cerr for python

	// TODO logging data to both a file and an endpoint

/// Data
private :
	bool _verbose;

	// old streambuffers so we can restore them later
	std::streambuf *_old_cout;
	std::streambuf *_old_cerr;

	std::string _output_filename;
	std::string _error_filename;

	std::vector<LogMessage> _messages;
	std::vector< io::stream_buffer<sink> *> _streams;

};	// namespace Logger

}	// namespace vl

#endif	// VL_LOGGER_HPP
