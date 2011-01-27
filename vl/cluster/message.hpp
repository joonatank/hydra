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

	void read( char *mem, size_t size )
	{
		::memcpy( mem, &_data[0], size );
		_size -= size;
		_data.erase( _data.begin(), _data.begin()+size );
	}

	/// Write an arbitary object to the message data, this never writes the header or size
	template<typename T>
	void write( T const &obj );

	void write( char const *mem, size_t size )
	{
		_data.resize( _data.size()+size );
		::memcpy( &_data[_size], mem, size );
		_size += size;
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

std::ostream &operator<<( std::ostream &os, Message const &msg );

// TODO test if this can be overriden easily.
// If not replace with multiple non-template methods so that the compiler
// will catch errors if type is not supported.
/// Specialize the template if you need to use more complicated serialization
/// than straigth memory copy.
/// Problematic because you need to specialise it in namespace vl::cluster
/// and there will be no compiler error if you don't
/// (or forget to include the header which defines the specialisation).
template<typename T>
Message &operator<<( Message &msg, T const &t )
{
	msg.write(t);
	return msg;
}

template<typename T>
Message &operator<<( Message &msg, std::vector<T> const &v )
{
	msg.write( v.size() );
	for(size_t i = 0; i < v.size(); ++i )
	{ msg << v.at(i); }

	return msg;
}

template<typename T>
Message &operator>>( Message &msg, T &t )
{
	msg.read(t);
	return msg;
}

template<typename T>
Message &operator>>( Message &msg, std::vector<T> &v )
{
	size_t size;
	msg.read(size);
	v.resize( size );
	for(size_t i = 0; i < v.size(); ++i )
	{ msg >> v.at(i); }

	return msg;
}


template<typename T>
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

template<typename T>
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
// 	std::cout << "Message::read : Reading std::string" << std::endl;

	size_t size;
	::memcpy( &size, &_data[0], sizeof(size_t) );
	str.resize(size);
	for( size_t i = 0; i < size; ++i )
	{
		str.at(i) = _data[i+sizeof(size_t)];
	}
	_data.erase( _data.begin(), _data.begin()+size+sizeof(size_t) );
	_size -= ( size + sizeof(size_t) );

// 	 std::cout << "string = " << str << std::endl;
}

template<> inline
void Message::write( std::string const &str )
{
// 	std::cout << "Message::write : Writing std::string = " << str << std::endl;

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