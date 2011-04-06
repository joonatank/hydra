/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-01
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

/**	Message Structures
 *
 *	Acknowledge a message received
 *	MSG_ACK
 *	[MSG_ACK, message type, id]
 *	message type is the type of message the client is acknowledging
 *	id is the id of the message, this is optional and not used at the moment
 *
 *	Reguest SceneGraph update messages
 *	MSG_REG_UPDATES
 *	[MSG_REG_UPDATES]
 *
 *	Add to the rendering group, 
 *	this should be sent only after initalisation is done
 *	avoids blocking for clients that are not ready to render
 *	and also provides the possibility of receiving updates but not rendering messages
 *	MSG_REG_RENDERING
 *
 *	Update message
 *	MSG_SG_UPDATE
 *	[MSG_SG_UPDATE, data size, [N | object id, object size, object data]]
 *	where one object is an versioned object (registered and can be mapped)
 *
 *	Create message
 *	MSG_SG_CREATE
 *	[MSG_SG_CREATE, data size, [N | object type id, object id]]
 *	where object type id is the id of the object to create
 *
 *	Input message
 *	MSG_INPUT
 *	[MSG_INPUT, data size, [N | input device type, input device id, event size, event data]]
 *	input device type : which kind of device joystick, keyboard, mouse
 *	input device id : every device has unique id
 *	event is the description of the event
 *
 *	Draw message
 *	MSG_DRAW
 *	[MSG_DRAW]
 *
 *	Swap message
 *	MSG_SWAP
 *	[MSG_SWAP]
 *
 *	Command message
 *	MSG_COMMAND
 *	[MSG_COMMAND | string command]
 *
 *	Log or output message
 *	MSG_PRINT
 *	[MSG_PRINT | uint16_t log_stream | std::string log_message]
 *
 *	Shutdown message
 *	MSG_SHUTDOWN
 *	[MSG_SHUTDOWN]
 *
 */
enum MSG_TYPES
{
	MSG_UNDEFINED = 0,	// Not defined message type these should never be sent
	MSG_ACK,			// Acknowledgement message
	MSG_REG_UPDATES,	// Reguest updates from the application
	MSG_REG_RENDERING,	// Add to the rendering group, 
	MSG_ENVIRONMENT,	// Send the Environment configuration
	MSG_PROJECT,		// Send the project configuration
	MSG_SG_CREATE,		// Create SceneGraph elements
	MSG_FRAME_START,	// Send from master to start a frame loop
	MSG_SG_UPDATE_READY,
	MSG_SG_UPDATE,		// Send updated SceneGraph
	MSG_SG_UPDATE_DONE,	// NOT IN USE
	MSG_INPUT,			// Send data from input devices from pipes to application
	MSG_DRAW_READY,		// Sent from Rendering thread when it's ready to draw
	MSG_DRAW,			// Draw the image into back buffer
	MSG_DRAW_DONE,
//	MSG_READY_SWAP,		// Sent from Rendering thread when it's ready to swap
//	MSG_SWAP,			// Swap the Window buffer
	MSG_COMMAND,		// Command string sent to Application
	MSG_REG_OUTPUT,		// Request logger output messages
	MSG_PRINT,			// Logger output messages, sent from application to pipes
	MSG_SHUTDOWN,		// Shutdown the rendering threads
};

enum EVENT_TYPES
{
	EVT_UNDEFINED,
	EVT_KEY_PRESSED,
	EVT_KEY_RELEASED,
	EVT_MOUSE_PRESSED,
	EVT_MOUSE_RELEASED,
	EVT_MOUSE_MOVED,
	EVT_JOYSTICK_PRESSED,
	EVT_JOYSTICK_RELEASED,
	EVT_JOYSTICK_AXIS,
	EVT_JOYSTICK_POV,
	EVT_JOYSTICK_VECTOR3,
};

inline
std::string getTypeAsString( MSG_TYPES type )
{
	switch( type )
	{
	case MSG_UNDEFINED :
		return "MSG_UNDEFINED";
	case MSG_ACK :
		return "MSG_ACK";
	case MSG_REG_UPDATES :
		return "MSG_REG_UPDATES";
	case MSG_REG_RENDERING:
		return "MSG_REG_RENDERING";
	case MSG_ENVIRONMENT :
		return "MSG_ENVIRONMENT";
	case MSG_PROJECT :
		return "MSG_PROJECT";
	case MSG_SG_CREATE :
		return "MSG_SG_CREATE";
	case MSG_FRAME_START:
		return "MSG_FRAME_START";
	case MSG_SG_UPDATE_READY :
		return "MSG_SG_UPDATE_READY";
	case MSG_SG_UPDATE :
		return "MSG_SG_UPDATE";
	case MSG_SG_UPDATE_DONE :
		return "MSG_SG_UPDATE_DONE";
	case MSG_INPUT :
		return "MSG_INPUT";
	case MSG_DRAW_READY :
		return "MSG_DRAW_READY";
	case MSG_DRAW :
		return "MSG_DRAW";
	case MSG_DRAW_DONE :
		return "MSG_DRAW_DONE";
	case MSG_COMMAND :
		return "MSG_COMMAND";
	case MSG_REG_OUTPUT:
		return "MSG_REG_OUTPUT";
	case MSG_PRINT :
		return "MSG_PRINT";
	case MSG_SHUTDOWN :
		return "MSG_SHUTDOWN";
	default :
		return std::string();
	}
}

/// Description of an UDP message
class Message
{
public :
	typedef uint32_t size_type;

	Message( std::vector<char> const &arr);

	Message( MSG_TYPES type = MSG_UNDEFINED );

	MSG_TYPES getType( void ) const
	{ return _type; }

	/// Dump the whole message to a binary array, the array is modified
	virtual void dump( std::vector<char> &arr ) const;

	bool empty(void) const;

	void clear(void);

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
	size_type size( void ) const
	{ return _size; }

	friend std::ostream &operator<<( std::ostream &os, Message const &msg );

private :
	/**	@todo Replace all the message data with one std::vector<char>
	 *	removes copying when the message is dumped for real sending
	 *	anyway message will
	 *	HUH, doesn't make sense if we use different class to create the
	 *	real message so that we can support message splitting.
	 *	That message data should be completely in std::vector<char>
	 */
	MSG_TYPES _type;
	size_type _size;
	std::vector<char> _data;

};	// class Message

std::ostream &operator<<( std::ostream &os, Message const &msg );

/** @class MessagePart
 *	@brief A class used for splitting and reconstruction of Messages
 *	@todo Not in use at the moment
 *
 *	Messages should read/write to multiple MessageParts which are sent
 *	separately.
 */
class MessagePart
{
public :
	MessagePart( void );

	MSG_TYPES getType( void ) const
	{ return _type; }

	Message::size_type start_index( void ) const
	{ return _start_index; }

	Message::size_type end_index( void ) const
	{ return _end_index; }

	Message::size_type size( void ) const
	{ return _end_index-_start_index; }

	/// Dump the whole message to a binary array, the array is modified
	virtual void dump( std::vector<char> &arr ) const;

	/**	@brief If the message is valid for reading or is partial
	 *	@return True if message can be read and no data is missing, false otherwise
	 *
	 *	valid() = !partial() always
	 */
	bool valid( void ) const;

	bool partial( void ) const;

private :
	MSG_TYPES _type;
	Message::size_type _start_index;
	Message::size_type _end_index;
	Message::size_type _whole_size;
	std::vector<char> _data;
};

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
	{ read( (char *)&t, (msg_size)sizeof(t) ); }

	template<typename T>
	void write( T const &t )
	{ write( (char *)&t, (msg_size)sizeof(t) ); }

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
ByteStream &operator>>( ByteStream &msg, T &t )
{
	msg.read( (char *)&t, (vl::msg_size)sizeof(t) );
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
ByteStream &operator>>( ByteStream &msg, std::vector<T> &v )
{
	typename std::vector<T>::size_type size;
	msg >> size;
	v.resize( size );
	for(size_t i = 0; i < v.size(); ++i )
	{ msg >> v.at(i); }

	return msg;
}

template<> inline
ByteStream &operator<<( ByteStream &msg, std::string const &str )
{
	msg.write( str.size() );
	if( str.size() != 0 )
	{ msg.write( str.c_str(), str.size() ); }

	return msg;
}

template<> inline
ByteStream &operator>>( ByteStream &msg, std::string &str )
{
	std::string::size_type size;
	msg.read(size);
	if( 0 == size )
	{ str.clear(); }
	else
	{
		str.resize(size);
		for( size_t i = 0; i < size; ++i )
		{
			char ch;
			msg.read(ch);
			str.at(i) = ch;
		}
	}

	return msg;
}


// TODO are these necessary still, when using the ByteStream to write data?
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
msg_size Message::write(std::string const &str)
{
	write( str.size() );
	if( str.size() != 0 )
	{ write( str.c_str(), str.size() ); }

	return sizeof(str.size())+sizeof(std::string::value_type)*str.size();
}

template<> inline
msg_size Message::read(std::string &str)
{
	std::string::size_type size;
	read(size);
	if( 0 == size )
	{ str.clear(); }
	else
	{
		str.resize(size);
		for( size_t i = 0; i < size; ++i )
		{
			char ch;
			read(ch);
			str.at(i) = ch;
		}
	}

	return sizeof(size)+sizeof(std::string::value_type)*size;
}

}	// namespace cluster

}	// namespace vl

namespace std
{

/// Print byte data
std::ostream &operator<<( std::ostream &os, vector<char> const &v );

}

#endif // VL_CLUSTER_MESSAGE_HPP
