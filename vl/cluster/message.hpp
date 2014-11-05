/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-01
 *	@file cluster/message.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

#ifndef HYDRA_CLUSTER_MESSAGE_HPP
#define HYDRA_CLUSTER_MESSAGE_HPP

#include <stdint.h>

#include <vector>
// Necessary for memcpy
#include <cstring>

#include <iostream>

#include "base/exceptions.hpp"
#include "base/string_utils.hpp"

// Necessary for saving timestamps into Messages
#include "base/time.hpp"

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
	MSG_REQ_SG_UPDATE,	// Sent from slave to Master to request an SG update
	MSG_SG_INIT,		// Same as MSG_SG_UPDATE but sent before any other updates
	MSG_SG_UPDATE,		// Send updated SceneGraph
	MSG_SG_UPDATE_DONE,	// NOT IN USE
	MSG_INPUT,			// Send data from input devices from pipes to application
	MSG_DRAW_READY,		// Sent from Rendering thread when it's ready to draw
	MSG_DRAW,			// Draw the image into back buffer
	MSG_DRAWING,		// Slave started drawing
	MSG_DRAW_DONE,
//	MSG_READY_SWAP,		// Sent from Rendering thread when it's ready to swap
//	MSG_SWAP,			// Swap the Window buffer
	MSG_COMMAND,		// Command string sent to Application
	MSG_REG_OUTPUT,		// Request logger output messages
	MSG_PRINT,			// Logger output messages, sent from application to pipes
	MSG_SHUTDOWN,		// Shutdown the rendering threads
	MSG_REG_RESOURCE,	// Request a resource from Server
	MSG_RESOURCE,		// Resource message, resource can be anything
	MSG_INJECT_LAG,		// Test message that introduces an artificial lag
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
	EVT_JOYSTICK_SLIDER
};

enum RESOURCE_TYPE
{
	RES_MESH,
	RES_RECORDING,		// Recorded data that should be drawn
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
	case MSG_REQ_SG_UPDATE :
		return "MSG_REQ_SG_UPDATE";
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
	case MSG_REG_RESOURCE :
		return "MSG_REG_RESOURCE";
	case MSG_RESOURCE :
		return "MSG_RESOURCE";
	default :
		return std::string();
	}
}

/// @class ByteStream Abstract stream interface
class ByteStream
{
public :
	template<typename T>
	void read( T &t )
	{ read( (char *)&t, (msg_size)sizeof(t) ); }

	template<typename T>
	void write( T const &t )
	{ write( (char *)&t, (msg_size)sizeof(t) ); }

	virtual void read(char *mem, msg_size size) = 0;

	virtual void write(char const *mem, msg_size size) = 0;

};	// class ByteStream

class Message;

class MessageDataStream : public ByteStream
{
public :
	MessageDataStream(Message *msg)
		: _data(msg)
	{}

	virtual void read(char *mem, msg_size size);

	virtual void write(char const *mem, msg_size size);

private :
	Message *_data;

};	// class MessageDataStream

/// @brief copies a memory field to a member
/// @return true if successful, i.e. the char buffer has enough data
/// @param dest destination where to copy, this needs to be a primitive for templates to work
/// @param src char buffer where to copy the data
/// @param pos position variable, is updated to the new position after successful read
template<typename T, typename N>
bool copy_memory(T *dest, std::vector<char> const &src, N &pos)
{
	if( src.size() >= pos + sizeof(T) )
	{
		::memcpy( dest, &src[pos], sizeof(T) );
		pos += sizeof(T);
		return true;
	}
	else
	{ return false; }
}

const uint16_t MTU_SIZE = 1500;
const uint16_t UDP_HEADER_SIZE = 8;
const uint16_t IP_HEADER_SIZE = 20;
const uint16_t MSG_HEADER_SIZE = 4+8+2+2;
/// Size of a single message part, calculated from MTU and headers
/// so that a single message would pass undivided through the uplink.
/// @todo should be configurable (or overridable)
/// @todo should be checked that the datagram will really be 1500bytes not more.
const uint16_t MSG_PART_SIZE = 
	MTU_SIZE - (MSG_HEADER_SIZE+UDP_HEADER_SIZE+IP_HEADER_SIZE);

/*	Constant size message 
 *	[HEADER | DATA]
 *	Header:
 *	[TYPE (32bit) | ID (64bit) | PARTS (16bit) | PART (16bit) ]
 *	
 *	Data
 *	[DATA_SIZE (16bit) | DATA ]
 *
 *	@todo this should provide an interface for casting the Message into 
 *	memory buffers (asio or void *)
 *	Using asio buffers
 *	First buffer can be created from the header structure especially using 
 *	a separate struct for it (cast to void *).
 *	Second buffer can be created using a pointer to the Message structures
 *	Data part and an index for start data.
 */
struct MessagePart
{
	MessagePart(std::vector<char> const &buf);

	MessagePart(MSG_TYPES t, uint64_t id_, uint16_t parts_, uint16_t part_)
		: type(t), id(id_), parts(parts_), part(part_)
	{
		/// data size should be zero as long as there is no data in there
	}
	
	/// Constructs an undefined MessagePart, used for container resizes
	/// Do not use this!
	MessagePart(void)
		: type(vl::cluster::MSG_UNDEFINED)
		, id(0)
		, parts(0)
		, part(0)
	{}

	void dump(std::vector<char> &arr) const;

	size_t size(void) const;

	MSG_TYPES type;
	uint64_t id;
	uint16_t parts;
	uint16_t part;
	std::vector<char> data;
};

/** @class Message
 *	@brief Description of an UDP message
 *
 *	Stucture: [MESSAGE_TYPE | MESSAGE_ID | MESSAGE_PARTS | MESSAGE_PART | DATA_SIZE | DATA]
 *	MESSAGE_TYPE, MESSAGE_ID and MESSAGE_PARTS stay constant
 *	between multiple parts of the same message.
 *
 *	DATA_SIZE is the number of bytes in this part.
 *	DATA is some data that can be used to compose the complete message 
 *	depends on which part this is and what type of message.
 *
 *	When the parts are composed to a complete message
 *	[HEADER | DATA]
 *	Header
 *	[MESSAGE_TYPE | MESSAGE_ID | FRAME | TIMESTAMP]
 *	Data
 *	[DATA_SIZE | DATA]
 */
class Message
{
public :
	typedef uint32_t size_type;

	/// @constructor using the first part of the message
	Message(MessagePart const &part);

	Message(std::vector<MessagePart> const &parts);

	Message(MSG_TYPES type, uint32_t frame, vl::time const &timestamp);

	Message(void);

	void addPart(MessagePart const &part);

	std::vector<MessagePart> createParts(void) const;

	/// @brief is this message whole or is there a piece missing
	bool partial(void) const;

	MSG_TYPES getType( void ) const
	{ return _type; }

	uint64_t getID(void) const
	{ return _id; }

	uint32_t getFrame(void) const
	{ return _frame; }

	void setFrame(uint32_t frame)
	{ _frame = frame; }

	vl::time const &getTimestamp(void) const
	{ return _timestamp; }

	void setTimestamp(vl::time const &timestamp)
	{ _timestamp = timestamp; }

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
	{ return _data.size(); }

	MessageDataStream getStream(void)
	{ return MessageDataStream(this); }

	friend std::ostream &operator<<( std::ostream &os, Message const &msg );

	static uint64_t generateID(void);

private :

	static uint64_t _last_id;

	void _assemble(void);

	/// --------------------------- Data -------------------------------------
	/**	@todo Replace all the message data with one std::vector<char>
	 *	removes copying when the message is dumped for real sending
	 *	anyway message will
	 *	HUH, doesn't make sense if we use different class to create the
	 *	real message so that we can support message splitting.
	 *	That message data should be completely in std::vector<char>
	 */
	MSG_TYPES _type;
	uint64_t _id;
	/// Use map so that the parts are automatically sorted
	std::map<uint16_t, MessagePart> _parts;

	uint32_t _frame;
	vl::time _timestamp;

	std::vector<char> _data;

};	// class Message

std::ostream &operator<<( std::ostream &os, Message const &msg );

inline void
MessageDataStream::read(char *mem, msg_size size)
{
	assert(_data);
	_data->read(mem, size);
}

inline void
MessageDataStream::write(char const *mem, msg_size size)
{
	assert(_data);
	_data->write(mem, size);
}

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

/// This can be copied, copied stream contains the original object pointer
/// So it acts as a proxy for the object
class ByteDataStream : public ByteStream
{
public :
	ByteDataStream( ByteData *data )
		: _data(data)
	{ open(); }

	~ByteDataStream( void )
	{ close(); }

	void setData( ByteData *data )
	{
		close();
		_data = data;
		open();
	}

	void open(void)
	{
		if( _data )
		{ _data->open(); }
	}

	void close(void)
	{
		if( _data )
		{ _data->close(); }
	}

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

	ByteDataStream getStream( void )
	{ return ByteDataStream( this ); }

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

	ByteDataStream getStream( void )
	{ return ByteDataStream( this ); }

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

/// Pointers can not be serialised at the moment
/// These are working well, they will throw a compile error if they are used
template<typename T>
ByteStream &operator<<( ByteStream &msg, T const *t )
{
	static_assert(false, "Deserialising pointers is not allowed");
}

template<typename T>
ByteStream &operator>>( ByteStream &msg, T *t )
{
	static_assert(false, "Deserialising pointers is not allowed");
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
	typename std::vector<T>::size_type v_size;
	msg >> v_size;
	if(v_size > 0)
	{
		v.resize(v_size);
		for(size_t i = 0; i < v.size(); ++i )
		{ msg >> v.at(i); }
	}
	return msg;
}

// Specialisation for primitives that uses memcpy directly
// Deserializing element by element like the default implementation is 500 times slower
// for large objects so this is necessary.
// This implementation only works for primitive (C) types and not objects
// because it can not handle references obviously.
// Only provide implementation we need atm because every implementation needs
// to be separate i.e. we need different functions for 
// char, uint8_t, int16_t, uint16_t, int32_t, uint32_t etc.
template<> inline
ByteStream &operator<<( ByteStream &msg, std::vector<uint32_t> const &v )
{
	std::clog << "Vector of primitives to serialise." << std::endl;
	msg << v.size();
	
	size_t byte_size = sizeof(uint32_t)*v.size();
	std::clog << "Vector with " << v.size() << " members to serialise : "
		<< byte_size << " bytes = " << (vl::msg_size)byte_size << " bytes." << std::endl;


	msg.write( (char const *)&v[0], (vl::msg_size)(sizeof(uint32_t)*v.size()) );

	return msg;
}

template<> inline
ByteStream &operator>> ( ByteStream &msg, std::vector<uint32_t> &v )
{
	std::vector<std::string>::size_type v_size;
	msg >> v_size;
	//Checking needed, if vector is empty!
	if(v_size > 0) {
		v.resize(v_size);
		msg.read( (char *)&v[0], (vl::msg_size)(sizeof(uint32_t)*v_size) );
	}
	return msg;
}

/// Vector of pointers should never happen
/// These are working well, they will throw a compile error if they are used
template<typename T>
ByteStream &operator<<( ByteStream &msg, std::vector<T *> const &v )
{
	static_assert(false, "SHOULD NEVER HAPPEN : Vector of pointers to serialize.");
}

template<typename T>
ByteStream &operator>>( ByteStream &msg, std::vector<T *> &v )
{
	static_assert(false, "SHOULD NEVER HAPPEN : Vector of pointers to deserialise.");
}

// Some syntastical bshit these specialisations will collide with other specialisations
// without the explicit template parameter
template<> inline
ByteStream &operator<<(ByteStream &msg, std::string const &str)
{
	msg.write( str.size() );
	if( str.size() != 0 )
	{ msg.write( str.c_str(), str.size() ); }

	return msg;
}

template<> inline
ByteStream &operator>>(ByteStream &msg, std::string &str)
{
	std::string::size_type str_size;
	msg.read(str_size);
	if( 0 == str_size )
	{ str.clear(); }
	else
	{
		str.resize(str_size);
		for( size_t i = 0; i < str_size; ++i )
		{
			char ch;
			msg.read(ch);
			str.at(i) = ch;
		}
	}

	return msg;
}


/// These are used by the Stream classes for easier writing/reading
template<typename T>
msg_size Message::read(T& obj)
{
	msg_size size = sizeof(obj);
	if( _data.size() < size )
	{
		std::string str =
			std::string("vl::Message::read - Not enough data to read from Message. There is")
			+ vl::to_string(_data.size()) + " bytes : needs "
			+ vl::to_string(sizeof(obj)) + " bytes.";
		BOOST_THROW_EXCEPTION( vl::short_message() << vl::desc(str) );
	}

	::memcpy( &obj, &_data[0], size );
	_data.erase( _data.begin(), _data.begin()+size );

	return size;
}

template<typename T>
msg_size Message::write(const T& obj)
{
	size_type index = (size_type)_data.size();
	size_type size = sizeof(obj);
	_data.resize( index + size );
	::memcpy( &_data[0]+index, &obj, size );

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
/// Used by other printing functions
/// Needs to be in the std namespace otherwise it's not recognised by the compiler
std::ostream &operator<<( std::ostream &os, std::vector<char> const &v );

}	// namespace std

#endif // HYDRA_CLUSTER_MESSAGE_HPP
