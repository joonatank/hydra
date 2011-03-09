/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-03
 *	@file logger.cpp
 *
 */

#include "logger.hpp"

#include <iostream>
#include <fstream>

vl::sink::sink(vl::Logger& logger, LOG_TYPE type)
	: _logger(logger)
	, _type(type)
{}

std::streamsize
vl::sink::write(const char* s, std::streamsize n)
{
	std::string str(s, n);

	if( str != "\n" )
	{ _logger.logMessage(_type, str); }

	return n;
}

vl::Logger::Logger(void )
	: _verbose(false)
	, _old_cout(std::cout.rdbuf())
	, _old_cerr(std::cerr.rdbuf())
	, _output_filename("temp_log_cout.txt")
	, _error_filename("temp_log_cerr.txt")
{
	io::stream_buffer<sink> *s = new io::stream_buffer<sink>(*this, LOG_ERR);
	std::cerr.rdbuf(s);
	_streams.push_back(s);

	s = new io::stream_buffer<sink>(*this, LOG_OUT);
	std::cout.rdbuf(s);
	_streams.push_back(s);
}

vl::Logger::~Logger(void )
{
	// flush the the messages to files
	if( newMessages() )
	{
		std::ofstream file( _output_filename.c_str() );
		while( newMessages() )
		{
			file << popMessage();
		}
	}

	// restore old output buffer
	std::cout.rdbuf(_old_cout);
	std::cerr.rdbuf(_old_cerr);

	for( size_t i = 0; i < _streams.size(); ++i )
	{
		delete _streams.at(i);
	}
}

void
vl::Logger::logMessage(vl::LOG_TYPE type, std::string const &str)
{
	// TODO fix the time
	_messages.push_back( LogMessage(type, 0, str) );

	// TODO should put the message to the log file

	// TODO if verbose is on should print the message to console (old cout, cerr)
}

vl::LogMessage
vl::Logger::popMessage(void)
{
	if( !_messages.empty() )
	{
		LogMessage msg = _messages.front();
		_messages.erase(_messages.begin());
		return msg;
	}
	return LogMessage();
}
