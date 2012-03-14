/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-03
 *	@file logger.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 */

#include "logger.hpp"

#include <iostream>
#include <fstream>

namespace
{

std::string::iterator find_first_log_level(std::string &str)
{
	std::string::iterator err_iter = std::find(str.begin(), str.end(), vl::CRITICAL);
	std::string::iterator norm_iter = std::find(str.begin(), str.end(), vl::NORMAL);
	std::string::iterator trace_iter = std::find(str.begin(), str.end(), vl::TRACE);

	return std::min(std::min(err_iter, norm_iter), trace_iter);
}

std::pair<std::string, vl::LOG_MESSAGE_LEVEL> parse_log_message(std::string &str)
{
	std::string::iterator iter = find_first_log_level(str);
	std::string temp;
	vl::LOG_MESSAGE_LEVEL log_l = vl::LML_NORMAL;
	if( iter != str.end() )
	{
		if( *iter == vl::CRITICAL )
		{ log_l = vl::LML_CRITICAL; }
		else if( *iter == vl::TRACE )
		{ log_l = vl::LML_TRIVIAL; }

		str.erase(iter);
	}

	// TODO should not return the whole string should only return from start
	// to the next control character
	return std::make_pair(str, log_l);
}

}

/// class LogMessage
vl::LogMessage::LogMessage( std::string const &ty,
			double tim,
			std::string const &msg,
			LOG_MESSAGE_LEVEL lvl )
			: type(ty), time(tim), message(msg), level(lvl)
{
}

void
vl::LogMessage::append(std::string const &str)
{
	message.append(str);
}

void
vl::LogMessage::clear(void)
{
	message.clear();
}

bool
vl::LogMessage::empty(void) const
{
	return message.empty();
}

/// class sink
vl::sink::sink(vl::Logger& logger, std::string const &type)
	: _logger(logger)
	, _type(type)
{}

std::streamsize
vl::sink::write(const char* s, std::streamsize n)
{
	std::string str(s, n);
	std::pair<std::string, vl::LOG_MESSAGE_LEVEL> msg = parse_log_message(str);

	_logger.logMessage(_type, msg.first, msg.second);

	return n;
}

void
vl::sink::write(std::string const &str)
{
	write(str.c_str(), str.size());
}

vl::Logger::Logger(void )
	: _verbose(false)
	, _old_cout(std::cout.rdbuf())
	, _old_cerr(std::cerr.rdbuf())
	, _output_filename("temp_log_cout.txt")
{
	io::stream_buffer<sink> *s = addSink("ERROR");
	std::cerr.rdbuf(s);

	s = addSink("OUT");
	std::cout.rdbuf(s);

	addSink("PY_OUT");

	addSink("PY_ERROR");
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

void 
vl::Logger::setOutputFile( std::string const &filename )
{
	_output_filename = filename;
	// TODO should close the old file if it's open
}

io::stream_buffer<vl::sink> *
vl::Logger::addSink(std::string const &name)
{
	io::stream_buffer<sink> *s = new io::stream_buffer<sink>(*this, name);
	_streams.push_back(s);
	return s;
}

io::stream_buffer<vl::sink> *
vl::Logger::getSink(std::string const &name)
{
	for( size_t i = 0; i < _streams.size(); ++i )
	{
		if( name == (*_streams.at(i))->getType() )
		{
			return _streams.at(i);
		}
	}

	return 0;
}

io::stream_buffer<vl::sink> *
vl::Logger::getPythonOut(void)
{
	for( size_t i = 0; i < _streams.size(); ++i )
	{
		if( "PY_OUT" == (*_streams.at(i))->getType() )
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
		if( "PY_ERROR" == (*_streams.at(i))->getType() )
		{
			return _streams.at(i);
		}
	}

	return 0;
}

void
vl::Logger::logMessage(std::string const &type, std::string const &message, LOG_MESSAGE_LEVEL level)
{
	// filter annoying error report from CEGUI
	if(message == "Error prior to using GLSL Program Object : invalid operation"
		|| message == "Error prior to using GLSL Program Object : invalid value")
	{ return; }

	// @todo this does not take into account different types and levels
	if(!_current_msg.empty())
	{
		_current_msg.append(message);
	}
	else
	{
		// @todo fix the time
		_current_msg = LogMessage(type, 0, message, level);
	}

	// check for ready messages
	if(*(message.end()-1) == '\n' )
	{
		logMessage(_current_msg);
		_current_msg.clear();
	}
}

void
vl::Logger::logMessage(vl::LogMessage const &message)
{
	_messages.push_back(message);

	// Print all the logs into same file
	if( !_output_file.is_open() )
	{
		_output_file.open(_output_filename.c_str());
	}
	_output_file << message;

	// Message does not have line endings
	_output_file << std::endl;

	// TODO if verbose is on should print the message to console (old cout, cerr)
}

vl::LogMessage const &
vl::Logger::getMessage(size_t i) const
{
	return _messages.at(i);
}
