/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-01
 *	@file cluster/message.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */


#if defined _MSC_VER
// Disable conversion warnings, we do a lot of them, intentionally
#   pragma warning (disable : 4267)
#endif

#include "message.hpp"

vl::cluster::MessagePart::MessagePart(std::vector<char> const &buf)
{
	/// Read the header
	size_t offset = 0;
	::memcpy(&type, &buf[0]+offset, sizeof(MSG_TYPES));
	offset += sizeof(MSG_TYPES);
	::memcpy(&id, &buf[0]+offset, sizeof(uint64_t));
	offset += sizeof(uint64_t);
	::memcpy(&parts, &buf[0]+offset, sizeof(uint16_t));
	offset += sizeof(uint16_t);
	::memcpy(&part, &buf[0]+offset, sizeof(uint16_t));
	offset += sizeof(uint16_t);
	
	/// Data size is of 2 bytes because UDP datagram can hold 65536 bytes.
	uint16_t size;
	::memcpy(&size, &buf[0]+offset, sizeof(uint16_t));
	offset += sizeof(uint16_t);

	/// Is this true or is the buffer larger than this?
	if(offset+size > buf.size())
	{
		std::clog << "MessagePart::constructor : offset = " << offset 
			<< " size = " << size << " buffer size = " << buf.size() << std::endl;
		assert(false);
	}

	/// Read the data
	if(size > 0)
	{
		data.resize(size);
		::memcpy(&data[0], &buf[0]+offset, size);
	}
}

void
vl::cluster::MessagePart::dump(std::vector<char> &arr) const
{
	arr.resize(size());
	size_t pos = 0;
	::memcpy( &arr[0]+pos, &type, sizeof(type) );
	pos += sizeof(type);
	::memcpy( &arr[0]+pos, &id, sizeof(id) );
	pos += sizeof(id);
	::memcpy( &arr[0]+pos, &parts, sizeof(parts) );
	pos += sizeof(parts);
	::memcpy( &arr[0]+pos, &part, sizeof(part) );
	pos += sizeof(part);

	/// Write data
	uint16_t size_16 = data.size();
	::memcpy( &arr[0]+pos, &size_16, sizeof(size_16) );
	pos += sizeof(size_16);
	assert(arr.size() == pos + size_16);
	if(size_16 > 0)
	{ ::memcpy( &arr[0]+pos, &data[0], size_16 ); }
}


size_t 
vl::cluster::MessagePart::size(void) const
{
	size_t header_size = sizeof(type)+sizeof(id)+sizeof(part)+sizeof(parts);
	/// Data size is 16bits not 64bits (like size_t)
	size_t data_size = sizeof(uint16_t)+data.size();
	return header_size+data_size;
}

/// ---------------------------- Message ---------------------------------------
vl::cluster::Message::Message(vl::cluster::MessagePart const &part)
	: _type(vl::cluster::MSG_UNDEFINED)
	, _id(part.id)
	, _frame(0)
{
	addPart(part);
}

vl::cluster::Message::Message(std::vector<vl::cluster::MessagePart> const &parts)
	: _type(vl::cluster::MSG_UNDEFINED)
	, _id(0)
	, _frame(0)
{
	assert(!parts.empty());
	
	for(size_t i = 0; i < parts.size(); ++i)
	{
		addPart(parts.at(i));
	}
}

void 
vl::cluster::Message::addPart(vl::cluster::MessagePart const &part)
{
	/// Copy the id and type
	if(_parts.empty())
	{
		_type = part.type;
		_id = part.id;
	}
	/// Check that they belong to same message
	else
	{
		// Replace with throw
		assert(_type == part.type && _id == part.id);
	}

	/// @todo replace with throwing
	assert(_parts.find(part.part) == _parts.end());
	_parts[part.part] = part;

	/// Complete
	if(part.parts == _parts.size())
	{
		_assemble();
	}
}

bool 
vl::cluster::Message::partial(void) const
{
	if( _parts.empty() )
	{ return false; }
	else if(_parts.size() == _parts.begin()->second.parts)
	{ return false; }
	else
	{ return true; }
}

void
vl::cluster::Message::_assemble(void)
{
	assert(_data.empty());
	assert(!_parts.empty());

	size_type size_of_data = 0;
	size_t data_offset = 0;
	std::map<uint16_t, MessagePart>::iterator last_iter;
	for(size_t i = 0; i < _parts.size(); ++i)
	{
		std::map<uint16_t, MessagePart>::iterator iter = _parts.find(i);
		// No need to do assertions for the Message types, done already
		assert(iter != _parts.end());
		std::vector<char> const &part_data = iter->second.data;
		size_type part_offset = 0;

		/// Read the header values (only in first message part)
		if(i == 0)
		{
			// Copy frame
			bool retval = copy_memory(&_frame, part_data, part_offset);
			if( !retval )
			{ BOOST_THROW_EXCEPTION( vl::short_message() ); }

			// Copy timestamp
			retval = copy_memory(&_timestamp, part_data, part_offset);
			if( !retval )
			{ BOOST_THROW_EXCEPTION( vl::short_message() ); }

			// Copy size
			size_type size_32 = 0;
			retval = copy_memory(&size_32, part_data, part_offset);
			if( !retval )
			{ BOOST_THROW_EXCEPTION( vl::short_message() ); }
			_data.resize(size_32);
		}

		/// Read data
		size_type part_bytes_remaining = part_data.size()-part_offset;
		if( part_bytes_remaining  > 0 )
		{
			if(_data.size()-data_offset < part_bytes_remaining)
			{
				std::clog << "data.size() = " << _data.size()
					<< " data_offset = " << data_offset << " part_bytes_remaining = "
					<< part_bytes_remaining << std::endl;
				assert(false);
			}

			::memcpy( &_data[0]+data_offset, &part_data[0]+part_offset, part_bytes_remaining);
			part_offset += part_bytes_remaining;
			data_offset += part_bytes_remaining;
		}

		last_iter = iter;
	}

	_parts.clear();
}

std::vector<vl::cluster::MessagePart> 
vl::cluster::Message::createParts(void) const
{
	size_t bytes = sizeof(_frame)+sizeof(_timestamp)+sizeof(size_type)+_data.size();
	uint16_t n_parts = bytes/MSG_PART_SIZE;
	n_parts += bytes%MSG_PART_SIZE ? 1 : 0;

	std::vector<vl::cluster::MessagePart> parts;

	size_type msg_offset = 0;
	for(uint16_t i = 0; i < n_parts; ++i)
	{
		MessagePart part(_type, _id, n_parts, i);
		uint16_t part_offset = 0;
		
		/// Calculate part data size
		uint16_t data_size = 0;
		uint16_t header_size = 0;
		// header
		if(i == 0)
		{
			header_size += sizeof(_frame)+sizeof(_timestamp)+sizeof(size_type);
		}
		/// This is way too convoluded
		/// Either take the maximum size of the message minus header
		/// Or take the rest of the data.
		data_size = MSG_PART_SIZE-header_size < _data.size() - msg_offset
			? MSG_PART_SIZE-header_size : _data.size()-msg_offset;

		/// Reserve enough data
		/// @todo the size of the message should be calculated before writes
		/// so that the table can be preallocated
		part.data.resize(header_size + data_size);

		/// Copy header
		/// @todo the header should be writen/read by a separate function
		if(header_size)
		{
			::memcpy(&part.data[0]+part_offset, &_frame, sizeof(_frame));
			part_offset += sizeof(_frame);
			::memcpy(&part.data[0]+part_offset, &_timestamp, sizeof(_timestamp));
			part_offset += sizeof(_timestamp);
			size_type size_32 = _data.size();
			::memcpy(&part.data[0]+part_offset, &size_32, sizeof(size_type));
			part_offset += sizeof(size_type);
		}

		assert(part_offset == header_size);
		assert(header_size + data_size <= MSG_PART_SIZE);

		if(data_size > 0)
		{
			assert(part.data.size() == part_offset + data_size);
			::memcpy(&part.data[0]+part_offset, &_data[0]+msg_offset, data_size);
		}
		msg_offset += data_size;

		parts.push_back(part);
	}

	return parts;
}

vl::cluster::Message::Message( vl::cluster::MSG_TYPES type, uint32_t frame, vl::time const &timestamp )
	: _type(type)
	, _id(generateID())
	, _frame(frame)
	, _timestamp(timestamp)
{}

vl::cluster::Message::Message(void)
	: _type(MSG_UNDEFINED), _id(0), _frame(0), _timestamp(0)
{}

bool 
vl::cluster::Message::empty(void) const
{
	return _data.empty();
}

void
vl::cluster::Message::clear( void )
{
	_data.clear();
}

vl::msg_size
vl::cluster::Message::read( char *mem, vl::msg_size size )
{
	/// @todo replace with throw
	if(_data.size() < size)
	{
		// Seems like this throw is working fine for slaves so we don't need the priting
		// @todo this should also include the message size and needed size
		BOOST_THROW_EXCEPTION(vl::short_message() << vl::desc("not enough data in the message"));
	}
	::memcpy( mem, &_data[0], size );
	_data.erase( _data.begin(), _data.begin()+size );

	return size;
}

vl::msg_size
vl::cluster::Message::write( char const *mem, vl::msg_size size )
{
	size_type index = _data.size();
	_data.resize( index+size );
	::memcpy( &_data[0]+index, mem, size );

	return size;
}

uint64_t vl::cluster::Message::generateID(void)
{
	return ++_last_id;
}

uint64_t vl::cluster::Message::_last_id = 0;

/// ----------------------------- ObjectData -----------------------------------
vl::cluster::ObjectData::ObjectData( uint64_t id )
	: _id(id)
{}

void
vl::cluster::ObjectData::read( char *mem, vl::msg_size size )
{
	assert( mem );
	if( 0 == size )
	{ return; }

	if( _data.size() < size )
	{
		// @todo replace with real exception
		std::clog << "Trying to read " << size << " bytes : but only has "
			<< _data.size() << " bytes left in the Message." << std::endl;
		BOOST_THROW_EXCEPTION(vl::exception());
	}

	::memcpy( mem, &_data[0], size );
	_data.erase( _data.begin(), _data.begin()+size );
}

void
vl::cluster::ObjectData::write( char const *mem, vl::msg_size size )
{
	assert( mem );
	if( 0 == size )
	{ return; }

	size_t pos = _data.size();
	_data.resize( _data.size() + size );
	::memcpy( &_data[pos], mem, size);
}

void
vl::cluster::ObjectData::copyToMessage( vl::cluster::Message *msg )
{
	assert(msg);
	// TODO define invalid ID
	assert( _id != 0 );
	msg->write(_id);

	assert( _data.size() < msg_size(-1) );
	msg_size size = _data.size();
	msg->write(size);
	msg->write( &_data[0], size );
}

void
vl::cluster::ObjectData::copyFromMessage( vl::cluster::Message *msg )
{
	assert(msg);

	// Skip the last object if not wholly read
	// FIXME this should not remove those from the message but rather move
	// the pointer used for the next stream
	// The idea is that one instance of the Serializer handles the reading of
	// the whole message but all the remaining bytes are stored.
	// So that later another instance can read through all the remaining bytes.

	msg->read(_id);

	msg_size size;
	// Check that there is more data than the size
	assert( msg->size() >= sizeof(size) );
	msg->read(size);
	assert( msg->size() >= size );
	_data.resize(size);

	msg->read( &_data[0], size );
}

/// ----------------------------- EventData ------------------------------------
vl::cluster::EventData::EventData( vl::cluster::EVENT_TYPES event_type )
	: _type(event_type)
{}

void
vl::cluster::EventData::read( char *mem, vl::msg_size size )
{
	assert( mem );
	if( 0 == size )
	{ return; }

	assert( _data.size() >= size );

	::memcpy( mem, &_data[0], size );
	_data.erase( _data.begin(), _data.begin()+size );
}

void
vl::cluster::EventData::write( char const *mem, vl::msg_size size )
{
	assert( mem );
	if( 0 == size )
	{ return; }

	size_t pos = _data.size();
	_data.resize( _data.size() + size );
	::memcpy( &_data[pos], mem, size);
}

void
vl::cluster::EventData::copyToMessage( vl::cluster::Message *msg )
{
	assert(msg);
	// TODO define invalid ID
	assert( _type != 0 );
	msg->write(_type);

	assert( _data.size() < msg_size(-1) );
	msg_size size = _data.size();
	msg->write(size);
	msg->write( &_data[0], size );
}

void
vl::cluster::EventData::copyFromMessage( vl::cluster::Message *msg )
{
	assert(msg);

	msg->read(_type);

	msg_size size;
	// Check that there is more data than the size
	assert( msg->size() > sizeof(size)-1 );
	msg->read(size);
	assert( msg->size() > size-1 );
	_data.resize(size);

	msg->read( &_data[0], size );
}



/// ------------------------------ Global --------------------------------------
std::ostream &
vl::cluster::operator<<(std::ostream& os, vl::cluster::ObjectData const &data)
{
	os << "Object id = " << data._id << " : size = " << data._data.size()
		<< " : data = " << data._data << std::endl;

	return os;
}


std::ostream &
vl::cluster::operator<<( std::ostream &os, vl::cluster::Message const &msg )
{
	os << "Message : type = " << msg._type << " : size = " << msg._data.size()
		<< " data = ";
	for( size_t i = 0; i < msg._data.size(); ++i )
	{ os << (uint16_t)( msg._data.at(i) ); }
	os << std::endl;

	return os;
}

std::ostream &
std::operator<<(std::ostream& os, std::vector< char > const &v)
{
	for( size_t i = 0; i < v.size(); ++i )
	{ os << v[i]; }

	return os;
}
