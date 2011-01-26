/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2011-01
 */

#ifndef VL_CLUSTER_MESSAGE_HPP
#define VL_CLUSTER_MESSAGE_HPP

#include <stdint.h>

#include <vector>
// Necessary for memcpy
#include <cstring>

#include <iostream>

namespace vl
{

namespace cluster
{

enum MSG_TYPES
{
	MSG_UNDEFINED,
	MSG_REG_UPDATES,
	MSG_UPDATE
};

/// Description of an UDP message
/// Has constant data put different methods which allows smoothly casting the
/// message from one message type to another.
/// TODO is this a wise design decission?
class Message
{
public :
	Message( std::vector<char> const &arr)
		: _type(MSG_UNDEFINED), _size(0)
	{
		size_t pos = 0;
		if( arr.size() >= pos + sizeof(_type) )
		{
			::memcpy( &_type, &arr[pos], sizeof(_type) );
			pos += sizeof(_type);
		}
		else
		{ return; }

		if( arr.size() >= pos + sizeof(_size) )
		{
			::memcpy( &_size, &arr[pos], sizeof(_size) );
			pos += sizeof(_size);
		}
		else
		{ return; }

		if( _size > 0 )
		{
			size_t size = _size;
			if( arr.size() < pos + size )
			{
				std::cerr << "A message is partial!" << std::endl;
				size = arr.size() - pos;
			}

			_data.resize(_size);
			::memcpy( &_data[0], &arr[pos], size );
		}
	}

	Message( MSG_TYPES type )
		: _type(type), _size(0)
	{}

	MSG_TYPES getType( void )
	{ return _type; }

	/// Dump the whole message to a binary array, the array is modified
	virtual void dump( std::vector<char> &arr ) const
	{
		arr.resize( sizeof(_type)+sizeof(_size)+_data.size() );
		size_t pos = 0;
		::memcpy( &arr[pos], &_type, sizeof(_type) );
		pos += sizeof( _type );
		::memcpy( &arr[pos], &_size, sizeof(_size) );
		pos += sizeof(_size);
		if( _data.size() > 0 )
		{ ::memcpy( &arr[pos], &_data[0], _data.size() ); }
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
/*
		std::cout << "Message::read data size = " << _data.size() << " data = ";
		for( size_t i = 0; i < _data.size(); ++i )
		{
			std::cout << (uint16_t)(_data.at(i));
		}
		std::cout << std::endl;
*/
		if( _size < sizeof(obj) )
		{
			std::cerr << "Not enough data to read from Message." << std::endl;
			return;
		}

		::memcpy( &obj, &_data[0], sizeof(obj) );
		_data.erase( _data.begin(), _data.begin()+sizeof(obj) );
		_size -= sizeof(obj);
	}

	/// Write an arbitary object to the message data, this never writes the header or size
	template<typename T>
	void write( T const &obj )
	{
// 		size_t size = _data.size();
		_data.resize( _data.size() + sizeof(obj) );
// 		std::cout << "Message::write : data size = " << _data.size();
		::memcpy( &_data[_size], &obj, sizeof(obj) );
// 		std::cout << " new size = " << _data.size() << " should have increased"
// 			<< " by " << sizeof(obj) << " bytes." << std::endl;
		_size += sizeof(obj);
// 		std::cout << "Writen object = " << obj << " data = ";
// 		for( size_t i = 0; i < _data.size(); ++i )
// 		{ std::cout << (uint16_t)(_data.at(i)); }
// 		std::cout << std::endl;
	}

	/// Size of the message in bytes
	/// Contains the message, not the type of the message which precedes the message
	/// Maximum size is 8kbytes, which is more than one datagram can handle
	/// For now larger messages are not supported
	uint16_t size( void )
	{ return _size; }

	friend std::ostream &operator<<( std::ostream &os, Message const &msg );
private :
	MSG_TYPES _type;
	uint16_t _size;
	std::vector<char> _data;

};	// class Message


inline
std::ostream &operator<<( std::ostream &os, Message const &msg )
{
	os << "Message : type = " << msg._type << " : size = " << msg._size
		<< " data = ";
	for( size_t i = 0; i < msg._data.size(); ++i )
	{ os << (uint16_t)( msg._data.at(i) ); }
	os << std::endl;

	return os;
}

}	// namespace cluster

}	// namespace vl

#endif // VL_CLUSTER_MESSAGE_HPP