#include "print_command.hpp"

#include <algorithm>
#include <iostream>

#include "base/print.hpp"

vl::udp::PrintCommand::PrintCommand( std::string const &typ, std::string const &obj )
	: _object(obj)
{
	std::string str(typ);
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	if( str == "setposition" )
	{
		_type = CMD_POS;
		_data.resize(3);
	}
	else if( str == "setquaternion" )
	{
		_type = CMD_ROT_QUAT;
		_data.resize(4);
	}
	else if( str == "setangle" )
	{
		_type = CMD_ROT_AA;
		_data.resize(4);
	}
}

vl::udp::PrintCommand::~PrintCommand( void )
{
}

void
vl::udp::PrintCommand::operator()( void )
{
	std::string cmd_str;
	if( _type == CMD_POS )
	{
		cmd_str = "set position";
	}
	else if( _type == CMD_ROT_QUAT )
	{
		cmd_str = "quaternion rotation";
	}
	if( _type == CMD_ROT_AA )
	{
		cmd_str = "angle-axis rotation";
	}

	std::cout << "Command " << cmd_str << " on object = " << _object
		<< " : with value = " << _data;
}

size_t
vl::udp::PrintCommand::getSize(void ) const
{
	if( _type == CMD_POS )
	{ return 3; }
	else if( _type == CMD_ROT_QUAT )
	{ return 4; }
	else if( _type == CMD_ROT_AA )
	{ return 4; }
	// We should only reach this if the CMD type is NONE
	else
	{ return 0; }
}

double &
vl::udp::PrintCommand::at( size_t i )
{ return _data.at(i); }

double const &
vl::udp::PrintCommand::at( size_t i ) const
{ return _data.at(i); }
