/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2011-01
 */

#include "message.hpp"


/// ---------------------------- Message ---------------------------------------
vl::cluster::Message::Message( std::vector<char> const &arr )
	: _type(vl::cluster::MSG_UNDEFINED), _size(0)
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

vl::cluster::Message::Message( vl::cluster::MSG_TYPES type )
	: _type(type), _size(0)
{}

void
vl::cluster::Message::dump( std::vector< char >& arr ) const
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

void
vl::cluster::Message::clear( void )
{
	_size = 0;
	_data.clear();
}

vl::msg_size
vl::cluster::Message::read( char *mem, vl::msg_size size )
{
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
