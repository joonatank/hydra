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

#include "base/exceptions.hpp"
#include "base/string_utils.hpp"

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
	Message( std::vector<char> const &arr);

	Message( MSG_TYPES type );


	MSG_TYPES getType( void )
	{ return _type; }

	/// Dump the whole message to a binary array, the array is modified
	virtual void dump( std::vector<char> &arr ) const;

	void clear( void );

	/// Read an arbitary type from message data, this never reads the header or size
	template<typename T>
	void read( T &obj );

	/// Write an arbitary object to the message data, this never writes the header or size
	template<typename T>
	void write( T const &obj );

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

std::ostream &operator<<( std::ostream &os, Message const &msg );


template<typename T> inline
void Message::read(T& obj)
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
		std::string str =
			std::string("vl::Message::read - Not enough data to read from Message. There is")
			+ vl::to_string(_size) + " bytes : needs "
			+ vl::to_string(sizeof(obj)) + " bytes.";
		BOOST_THROW_EXCEPTION( vl::short_message() << vl::desc(str) );
		return;
	}

	::memcpy( &obj, &_data[0], sizeof(obj) );
	_data.erase( _data.begin(), _data.begin()+sizeof(obj) );
	_size -= sizeof(obj);
}

template<typename T> inline
void Message::write(const T& obj)
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

template<> inline
void Message::read( std::string &str )
{
// 	std::cout << "Message::read : Reading std::string." << std::endl;

	size_t size;
	::memcpy( &size, &_data[0], sizeof(size_t) );
	str.resize(size);
	for( size_t i = 0; i < size; ++i )
	{
		str.at(i) = _data[i+sizeof(size_t)];
	}
	_data.erase( _data.begin(), _data.begin()+size+sizeof(size_t) );
	_size -= ( size + sizeof(size_t) );
}

template<> inline
void Message::write( std::string const &str )
{
// 	std::cout << "Message::write : Writing std::string." << std::endl;

	size_t size = str.size();
	_data.resize( _data.size() + size + sizeof(size_t) );
	::memcpy( &_data[_size], &size, sizeof(size_t) );
	_size += sizeof(size_t);
	::memcpy( &_data[_size], str.c_str(), size );
	_size += size;
}

}	// namespace cluster

}	// namespace vl

#endif // VL_CLUSTER_MESSAGE_HPP