/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-01
 */

#include "message.hpp"


/// ---------------------------- Message ---------------------------------------
vl::cluster::Message::Message( std::vector<char> const &arr )
	: _type(vl::cluster::MSG_UNDEFINED), _size(0), _frame(0)
{
	// @todo this should work with simple casting/memcpy
	// but do we want to use those, as it will break the user interface

	size_type pos = 0;
	// Copy type
	bool retval = _copy_memory(&_type, arr, pos);
	if( !retval )
	{ BOOST_THROW_EXCEPTION( vl::short_message() ); }

	// Copy frame
	retval = _copy_memory(&_frame, arr, pos);
	if( !retval )
	{ BOOST_THROW_EXCEPTION( vl::short_message() ); }

	// Copy timestamp
	retval = _copy_memory(&_timestamp, arr, pos);
	if( !retval )
	{ BOOST_THROW_EXCEPTION( vl::short_message() ); }

	// Copy size
	retval = _copy_memory(&_size, arr, pos);
	if( !retval )
	{ BOOST_THROW_EXCEPTION( vl::short_message() ); }

	if( _size > 0 )
	{
		size_t size = _size;
		if( arr.size() < pos + size )
		{
			// Appending messages is not supported
			BOOST_THROW_EXCEPTION( vl::short_message() );
		}

		_data.resize(size);
		::memcpy( &_data[0], &arr[pos], size );
	}
}

vl::cluster::Message::Message( vl::cluster::MSG_TYPES type, uint32_t frame, vl::time const &timestamp )
	: _type(type), _size(0), _frame(frame), _timestamp(timestamp)
{}

vl::cluster::Message::Message(void)
	: _type(MSG_UNDEFINED), _size(0), _frame(0), _timestamp(0)
{}

void
vl::cluster::Message::dump( std::vector< char >& arr ) const
{
	// this should work with just
	// @todo no it doesn't on multi node systems, for some reason
	// then again it's not symmetric if we use a struct memcpy here and
	// single memcpys in parsing the message
	// so the changes of something going wrong with reorganize or adding
	// more data are really high
//	arr.resize(sizeof(*this));
//	::memcpy( &arr[0], this, sizeof(*this) );

	arr.resize( sizeof(_type)+sizeof(_frame)+sizeof(_timestamp)+sizeof(_size)+_data.size() );
	size_t pos = 0;
	::memcpy( &arr[pos], &_type, sizeof(_type) );
	pos += sizeof( _type );
	::memcpy( &arr[pos], &_frame, sizeof(_frame) );
	pos += sizeof( _frame );
	::memcpy( &arr[pos], &_timestamp, sizeof(_timestamp) );
	pos += sizeof(_timestamp);
	::memcpy( &arr[pos], &_size, sizeof(_size) );
	pos += sizeof(_size);
	if( _data.size() > 0 )
	{ ::memcpy( &arr[pos], &_data[0], _data.size() ); }
}

bool 
vl::cluster::Message::empty(void) const
{
	return _size == 0 && _data.empty();
}

void
vl::cluster::Message::clear( void )
{
	_size = 0;
	_data.clear();
}

vl::msg_size
vl::cluster::Message::read( char *mem, vl::msg_size size )
{
	/// @todo replace with throw
	assert(_data.size() >= size);
	::memcpy( mem, &_data[0], size );
	_size -= size;
	_data.erase( _data.begin(), _data.begin()+size );

	return size;
}

vl::msg_size
vl::cluster::Message::write( char const *mem, vl::msg_size size )
{
	_data.resize( _data.size()+size );
	::memcpy( &_data[_size], mem, size );
	_size += size;

	return size;
}

/// -------------------------- MessagePart -----------------------------------
bool
vl::cluster::MessagePart::valid( void ) const
{
	return !partial();
}

bool
vl::cluster::MessagePart::partial( void ) const
{
	return size() != _data.size();
}

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

// 	std::cout << "vl::cluster::ObjectData::read : _data.size() = "
// 		<< _data.size() << " bytes : trying to read = " << size << " bytes."
// 		<< std::endl;
	assert( _data.size() >= size );

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
// 	std::cout << "vl::cluster::ObjectData::copyFromMessage" << std::endl;
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
	assert( msg->size() > sizeof(size)-1 );
	msg->read(size);
	assert( msg->size() > size-1 );
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
	os << "Message : type = " << msg._type << " : size = " << msg._size
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
