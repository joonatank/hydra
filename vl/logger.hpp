/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011/10 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-03
 *	@file logger.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef HYDRA_LOGGER_HPP
#define HYDRA_LOGGER_HPP

#include <streambuf>
#include <sstream>
#include <fstream>

#include <iosfwd>							// streamsize
#include <boost/iostreams/categories.hpp>	// sink_tag
#include <boost/iostreams/concepts.hpp>		// sink
#include <boost/iostreams/stream.hpp>

#include <vector>

#include "base/thread.hpp"

namespace io = boost::iostreams;

namespace vl
{

enum LOG_MESSAGE_LEVEL
{
	LML_CRITICAL = 0,
	LML_NORMAL,
	LML_TRIVIAL,
};

char const CRITICAL = 17;
char const NORMAL = 18;
char const TRACE = 19;

struct LogMessage
{
LogMessage( std::string const &ty = std::string(),
			double tim = 0,
			std::string const &msg = std::string(),
			LOG_MESSAGE_LEVEL lvl = LML_CRITICAL )
	: type(ty), time(tim), message(msg), level(lvl)
{
	if( message.size() > 0 )
	{
		// Remove the line ending because we are using custom line endings
		if(*(message.end()-1) == '\n')
		{ message.erase(message.end()-1); }
	}
}

std::string type;
double time;
std::string message;
LOG_MESSAGE_LEVEL level;

};	// class LogMessage

inline std::ostream &
operator<<(std::ostream &os, LogMessage const &msg)
{
	std::string level;
	if( msg.level == LML_CRITICAL )
	{ level = "CRITICAL"; }
	else if( msg.level == LML_NORMAL )
	{ level = "NORMAL"; }
	else if( msg.level == LML_TRIVIAL )
	{ level = "TRIVIAL"; }

	os << msg.type << "\t" << level << '\t' << msg.time << "s " << msg.message;
	return os;
}

/// Abstract class which defines the interface for sending LoggedMessages
class LogReceiver
{
public :
	virtual ~LogReceiver(void) {}

	virtual bool logEnabled(void) const = 0;

	virtual void logMessage(LogMessage const &msg) = 0;

	virtual uint32_t nLoggedMessages(void) const = 0;

};

class Logger;

class sink : public boost::iostreams::sink
{
public :
	sink( Logger &logger, std::string const &type );

	std::streamsize write(const char* s, std::streamsize n);

	void write(std::string const &str);

	std::string getType( void ) const
	{ return _type; }

	Logger &getLogger( void )
	{ return _logger; }

private :
	Logger &_logger;
	std::string _type;
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

	void setOutputFile( std::string const &filename );

	io::stream_buffer<sink> *addSink(std::string const &name);

	io::stream_buffer<sink> *getSink(std::string const &name);

	io::stream_buffer<sink> *getPythonOut( void );

	io::stream_buffer<sink> *getPythonErr( void );

	void logMessage( std::string const &type, std::string const &message, LOG_MESSAGE_LEVEL level = LML_CRITICAL );

	void logMessage(LogMessage const &message);

	size_t nMessages(void) const
	{ return _messages.size(); }

	LogMessage const &getMessage(size_t i) const;

/// Data
private :

	bool _verbose;

	// old streambuffers so we can restore them later
	std::streambuf *_old_cout;
	std::streambuf *_old_cerr;

	std::string _output_filename;
	std::ofstream _output_file;

	std::vector<LogMessage> _messages;
	std::vector< io::stream_buffer<sink> *> _streams;

	mutable vl::mutex _mutex;

};	// namespace Logger

}	// namespace vl

#endif	// HYDRA_LOGGER_HPP
