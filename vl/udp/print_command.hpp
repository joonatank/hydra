/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
 *
 */
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

	virtual std::vector<double> &operator<<( std::vector<double> &vec );

	virtual void print( std::ostream &os ) const;

private :
	
	std::string _object;
	
	std::vector<double> _data;

	CMD_TYPE _type;

};	// class PrintCommand

}	// namespace udp

}	// namespace vl

#endif