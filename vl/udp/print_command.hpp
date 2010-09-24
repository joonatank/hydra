#ifndef VL_UDP_PRINT_COMMAND_HPP
#define VL_UDP_PRINT_COMMAND_HPP

#include "command.hpp"

#include <string>
#include <vector>

namespace vl
{
	
namespace udp
{
	
class PrintCommand : public udp::Command
{
public :
	PrintCommand( std::string const &typ, std::string const &obj );

	virtual ~PrintCommand( void );

	virtual void operator()( void );

	virtual size_t getSize( void ) const;

	virtual CMD_TYPE getType( void ) const
	{ return _type; }

	virtual double &at( size_t i );

	virtual double const &at( size_t i ) const;

	virtual std::vector<double> &operator<<( std::vector<double> &vec )
	{ return vec; }
private :
	std::string _object;

	std::vector<double> _data;

	CMD_TYPE _type;

};	// class PrintCommand

}	// namespace udp

}	// namespace vl

#endif