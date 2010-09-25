#ifndef VL_UDP_COMMAND_HPP
#define VL_UDP_COMMAND_HPP

#include <stdint.h>
#include <cstring>
#include <vector>
#include <iostream>

#include <boost/shared_ptr.hpp>

namespace vl
{
	
namespace udp
{
	
enum CMD_TYPE
{
	CMD_NONE,
	CMD_POS,		// Set position, uses vector
	CMD_ROT_QUAT,	// Set rotation with quaternion
	CMD_ROT_AA,		// Set rotation with angle_axis
	CMD_ROT_ANGLE	// Set rotation with angle (axis has to be preset)
};

class Command
{
public :
	/// Execute the command
	virtual void operator()( void ) = 0;

	/// Returns the number of elements in the array for this command
	/// All elements are type double for now.
	virtual size_t getSize( void ) const = 0;

	virtual CMD_TYPE getType( void ) const = 0;

	/// Deserialization function
	/// syntax : cmd << vector
	/// Removes the read elements from the vector
	/// Throws if the input vector contains to few elements
	virtual std::vector<double> &operator<<( std::vector<double> &vec ) = 0;

	/// Real print function
	/// Called from operator<<( std::ostream &, Command const & ) to get virtual
	/// serialization to ostreams working correctly with nice syntax.
	virtual void print( std::ostream &os ) const = 0;
};	// class Command

typedef boost::shared_ptr<Command> CommandRefPtr;

inline
std::ostream & operator<<( std::ostream &os, Command const &cmd )
{
	cmd.print( os );
	return os;
}

}	// namespace udp

}	// namespace vl

#endif