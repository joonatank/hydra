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

void
vl::sink::write(std::string const &str)
{
	if( str != "\n" )
	{ _logger.logMessage(_type, str); }
}

vl::Logger::Logger(void )
	: _verbose(false)
	, _old_cout(std::cout.rdbuf())
	, _old_cerr(std::cerr.rdbuf())
	, _output_filename("temp_log_cout.txt")
{
	io::stream_buffer<sink> *s = new io::stream_buffer<sink>(*this, LOG_ERR);
	std::cerr.rdbuf(s);
	_streams.push_back(s);

	s = new io::stream_buffer<sink>(*this, LOG_OUT);
	std::cout.rdbuf(s);
	_streams.push_back(s);

	s = new io::stream_buffer<sink>(*this, LOG_PY_OUT);
	_streams.push_back(s);

	s = new io::stream_buffer<sink>(*this, LOG_PY_ERR);
	_streams.push_back(s);
}

vl::Logger::~Logger(void )
{
	// restore old output buffer
	std::cout.rdbuf(_old_cout);
	std::cerr.rdbuf(_old_cerr);

	for( size_t i = 0; i < _streams.size(); ++i )
	{
		delete _streams.at(i);
	}
}

io::stream_buffer<vl::sink> *
vl::Logger::getPythonOut(void)
{
	for( size_t i = 0; i < _streams.size(); ++i )
	{
		if( LOG_PY_OUT == (*_streams.at(i))->getType() )
		{
			return _streams.at(i);
		}
	}

	return 0;
}

io::stream_buffer<vl::sink> *
vl::Logger::getPythonErr(void)
{
	for( size_t i = 0; i < _streams.size(); ++i )
	{
		if( LOG_PY_ERR == (*_streams.at(i))->getType() )
		{
			return _streams.at(i);
		}
	}

	return 0;
}

void
vl::Logger::logMessage(vl::LOG_TYPE type, std::string const &str)
{
	// TODO fix the time
	LogMessage msg(type, 0, str);
	_messages.push_back(msg);

	// Print all the logs into same file
	if( !_output_file.is_open() )
	{
		_output_file.open(_output_filename.c_str());
	}
	_output_file << msg;
	// Workaround for lines without end line (Ogre logger)
	if(str.at(str.size()-1) != '\n')
	{ _output_file << std::endl; }

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
