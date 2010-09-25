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
//	this->operator<<(std::cout) << std::endl;
	std::cout << *this << std::endl;
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

std::vector< double > &
vl::udp::PrintCommand::operator<<(std::vector< double >& vec)
{
	for( size_t i = 0; i < _data.size(); ++i )
	{
		_data.at(i) = vec.at(i);
	}

	vec.erase( vec.begin(), vec.begin()+_data.size() );
	
	return vec;
}

void
vl::udp::PrintCommand::print( std::ostream& os ) const
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

	os << "PrintCommand " << cmd_str << " on object = " << _object
		<< " : with value = ";
	// Something funky with the operator<< for vectors we need to use this obscure syntax
	::operator<<(os, _data);
}