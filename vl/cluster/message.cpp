/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2011-01
 */

#include "message.hpp"

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

/*
std::ostream &
vl::cluster::operator<<(std::ostream& os, const vl::cluster::Message& msg)
{

}*/

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
