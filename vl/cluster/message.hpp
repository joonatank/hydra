/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2011-01
 */

#ifndef VL_CLUSTER_MESSAGE_HPP
#define VL_CLUSTER_MESSAGE_HPP

#include <stdint.h>

#include <vector>
// Necessary for memcpy
#include <cstring>

namespace vl
{

namespace cluster
{

enum MSG_TYPES
{
	REG_UPDATES,
	UPDATE
};

/// Description of an UDP message
/// Has constant data put different methods which allows smoothly casting the
/// message from one message type to another.
/// TODO is this a wise design decission?
class Message
{
public :
	Message( MSG_TYPES type )
		: _type(type)
	{}

	MSG_TYPES getType( void )
	{ return _type; }

	/// Read the whole message from a binary array, the array is modified
	virtual void create( std::vector<char> &arr )
	{
		::memcpy( &_size, &arr[0], 2 );
		arr.erase( arr.begin(), arr.begin()+1 );

		_data.resize(_size);
		for( uint16_t i = 0; i < _size; )
		{
			_data.at(i) = arr.at(i);
		}
		arr.erase( arr.begin(), arr.begin()+_size );
	}

	/// Dump the whole message to a binary array, the array is modified
	virtual void dump( std::vector<char> &arr )
	{
		arr.reserve( sizeof(_size)+sizeof(_data) );
		::memcpy( &arr[0], &_size, sizeof(_size) );
		::memcpy( &arr[sizeof(_size)], &_data[0], sizeof(_data) );
		// TODO should this clear the Message or not?
	}

	void clear( void )
	{
		_size = 0;
		_data.clear();
	}

	/// Read an arbitary type from message data, this never reads the header or size
	template<typename T>
	void read( T &obj )
	{
		::memcpy( &obj, &_data[0], sizeof(T) );
		_data.erase( _data.begin(), _data.begin()+sizeof(T) );
	}

	/// Write an arbitary object to the message data, this never writes the header or size
	template<typename T>
	void write( T const &obj )
	{
		_data.reserve( sizeof(T) );
		::memcpy( &_data[_data.size()], &obj, sizeof(T) );
	}

	/// Size of the message in bytes
	/// Contains the message, not the type of the message which precedes the message
	/// Maximum size is 8kbytes, which is more than one datagram can handle
	/// For now larger messages are not supported
	uint16_t size( void )
	{ return _size; }

private :
	MSG_TYPES _type;
	uint16_t _size;
	std::vector<char> _data;

};	// class Message

}	// namespace cluster

}	// namespace vl

#endif // VL_CLUSTER_MESSAGE_HPP