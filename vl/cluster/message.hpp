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

/// Message can hold 4Mbytes
typedef uint32_t msg_size;

namespace cluster
{

// TODO Add support for
// requesting initial settings, project configuration resources
// and responding to those requests
// TODO add support for ACK messages
/**	Message Structures
 *
 *	Reguest updates message
 *	MSG_REG_UPDATES
 *	[message type]
 *
 *	Update message
 *	both MSG_INITIAL_STATE and MSG_UPDATE
 *	[message type, data size, [N | object id, object size, object data]]
 *	where one object is an versioned object (registered and can be mapped)
 *
 *	Input message
 *	MSG_INPUT
 *	[message type, data size, [N | input device type, input device id, event size, event data]]
 *	input device type : which kind of device joystick, keyboard, mouse
 *	input device id : every device has unique id
 *	event is the description of the event
 *
 *	Draw message
 *	MSG_DRAW
 *	[message type]
 *
 *	Swap message
 *	MSG_SWAP
 *	[message type]
 *
 */
enum MSG_TYPES
{
	MSG_UNDEFINED,		// Not defined message type these should never be sent
	MSG_REG_UPDATES,	// Reguest updates from the application
	MSG_INITIAL_STATE,	// Send the initial SceneGraph
	MSG_UPDATE,	// Send updated SceneGraph and other versioned objects
	MSG_INPUT,	// Send data from input devices from pipes to application
	MSG_DRAW,	// Draw the image into back buffer
	MSG_SWAP	// Swap the Window buffer Not in use yet
};

enum EVENT_TYPES
{
	EVT_UNDEFINED,
	EVT_KEY_PRESSED,
	EVT_KEY_RELEASED,
	EVT_MOUSE_PRESSED,
	EVT_MOUSE_RELEASED,
	EVT_MOUSE_MOVED,
};

/// Description of an UDP message
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
	msg_size read( T &obj );

	msg_size read( char *mem, msg_size size );

	/// Write an arbitary object to the message data, this never writes the header or size
	template<typename T>
	msg_size write( T const &obj );

	msg_size write( char const *mem, msg_size size );

	char &operator[]( size_t index )
	{ return _data[index]; }

	char const &operator[]( size_t index ) const
	{ return _data[index]; }

	/// Size of the message in bytes
	/// Contains the message, not the type of the message which precedes the message
	/// Maximum size is 8kbytes, which is more than one datagram can handle
	/// For now larger messages are not supported
	msg_size size( void )
	{ return _size; }

	friend std::ostream &operator<<( std::ostream &os, Message const &msg );

private :
	MSG_TYPES _type;
	msg_size _size;
	std::vector<char> _data;

};	// class Message

class ByteData
{
public :
	virtual void read( char *mem, msg_size size ) = 0;
	virtual void write( char const *mem, msg_size size ) = 0;

	virtual void open( void ) = 0;
	virtual void close( void ) = 0;

	virtual void copyToMessage( Message *msg ) = 0;
	virtual void copyFromMessage( Message *msg ) = 0;

};	// class ByteData

// TODO break down to IStream and OStream
/// This can be copied, copied stream contains the original object pointer
/// So it acts as a proxy for the object
class ByteStream
{
public :
	ByteStream( ByteData *data )
		: _data(data)
	{ open(); }

	~ByteStream( void )
	{ close(); }

	void setData( ByteData *data )
	{
		close();
		_data = data;
		open();
	}

	void open( void )
	{
		if( _data )
		{ _data->open(); }
	}

	void close( void )
	{
		if( _data )
		{ _data->close(); }
	}

	template<typename T>
	void read( T &t )
	{ read( t, sizeof(t) ); }

	template<typename T>
	void write( T const &t )
	{ write( &t, sizeof(t) ); }

	virtual void read( char *mem, msg_size size )
	{
		assert( _data );
		_data->read(mem, size);
	}

	virtual void write( char const *mem, msg_size size )
	{
		assert( _data );
		_data->write(mem, size);
	}

private :
	ByteData *_data;

};	// class ByteStream

// TODO this should be constant when read
// The ByteStream should use an iterator or an index to read this structure
// without removing elements
class ObjectData : public ByteData
{
public :
	/// Invalid id is only valid for ObjectData that is read from message
	// TODO define invalid ID
	ObjectData( uint64_t id = 0);

	uint64_t getId( void ) const
	{ return _id; }

	void setId( uint64_t id )
	{ _id = id; }

	virtual void read( char *mem, msg_size size );

	virtual void write( char const *mem, msg_size size );

	virtual void open( void ) {}

	// Write the size of the object
	// TODO is this necessary _data has the size anyway?
	// No we can read/write the data using the _data array
	virtual void close( void ) {}

	virtual void copyToMessage( Message *msg );

	/// Message can contain multiple objects
	/// This function is used to separate different objects from each other
	/// When this is called it closes the message for receiving the next object
	/// And assumes that the next value writen is going to be the object id.
	/// Practically this function writes the amount of bytes used for each object
	/// as a next element to it's ID.
	virtual void copyFromMessage( Message *msg );

	ByteStream getStream( void )
	{ return ByteStream( this ); }

	friend std::ostream &operator<<( std::ostream &os, ObjectData const &data );

private :
	uint64_t _id;
	std::vector<char> _data;

};	// class ObjectData

std::ostream &operator<<( std::ostream &os, ObjectData const &data );

class EventData : public ByteData
{
public :
	EventData( EVENT_TYPES event_type = EVT_UNDEFINED);

	EVENT_TYPES getType( void ) const
	{ return _type; }

	void setType( EVENT_TYPES event_type )
	{ _type = event_type; }

	virtual void read( char *mem, msg_size size );
	virtual void write( char const *mem, msg_size size );

	virtual void open( void ) {}
	virtual void close( void ) {}

	virtual void copyToMessage( Message *msg );
	virtual void copyFromMessage( Message *msg );

	ByteStream getStream( void )
	{ return ByteStream( this ); }

private :
	std::vector<char> _data;
	EVENT_TYPES _type;
};

std::ostream &operator<<( std::ostream &os, Message const &msg );

/// Specialize the template if you need to use more complicated serialization
/// than straigth memory copy.
/// Problematic because you need to specialise it in namespace vl::cluster
/// and there will be no compiler error if you don't
/// (or forget to include the header which defines the specialisation).
template<typename T>
ByteStream &operator<<( ByteStream &msg, T const &t )
{
	msg.write( (char *)&t, (vl::msg_size)sizeof(t) );
	return msg;
}

template<typename T>
ByteStream &operator<<( ByteStream &msg, std::vector<T> const &v )
{
	msg << v.size();
	for(size_t i = 0; i < v.size(); ++i )
	{ msg << v.at(i); }

	return msg;
}

template<typename T>
ByteStream &operator>>( ByteStream &msg, T &t )
{
	msg.read( (char *)&t, (vl::msg_size)sizeof(t) );
	return msg;
}

template<typename T>
ByteStream &operator>>( ByteStream &msg, std::vector<T> &v )
{
	size_t size;
	msg >> size;
	v.resize( size );
	for(size_t i = 0; i < v.size(); ++i )
	{ msg >> v.at(i); }

	return msg;
}


template<typename T>
msg_size Message::read(T& obj)
{
	msg_size size = sizeof(obj);
	if( _size < size )
	{
		std::string str =
			std::string("vl::Message::read - Not enough data to read from Message. There is")
			+ vl::to_string(_size) + " bytes : needs "
			+ vl::to_string(sizeof(obj)) + " bytes.";
		BOOST_THROW_EXCEPTION( vl::short_message() << vl::desc(str) );
	}

	::memcpy( &obj, &_data[0], size );
	_data.erase( _data.begin(), _data.begin()+size );
	_size -= size;

	return size;
}

template<typename T>
msg_size Message::write(const T& obj)
{
	msg_size size = sizeof(obj);
	_data.resize( _data.size() + size );
	::memcpy( &_data[_size], &obj, size );
	_size += size;

	return size;
}

template<> inline
msg_size Message::read( std::string &str )
{
	size_t size;
	::memcpy( &size, &_data[0], sizeof(size_t) );
	str.resize(size);
	for( size_t i = 0; i < size; ++i )
	{
		str.at(i) = _data[i+sizeof(size_t)];
	}
	_data.erase( _data.begin(), _data.begin()+size+sizeof(size_t) );
	_size -= ( size + sizeof(size_t) );

	return size+sizeof(size_t);
}

template<> inline
msg_size Message::write( std::string const &str )
{
	size_t size = str.size();
	_data.resize( _data.size() + size + sizeof(size_t) );
	::memcpy( &_data[_size], &size, sizeof(size_t) );
	_size += sizeof(size_t);
	::memcpy( &_data[_size], str.c_str(), size );
	_size += size;

	return size + sizeof(size_t);
}

}	// namespace cluster

}	// namespace vl

namespace std
{

/// Print byte data
std::ostream &operator<<( std::ostream &os, vector<char> const &v );

}

#endif // VL_CLUSTER_MESSAGE_HPP
