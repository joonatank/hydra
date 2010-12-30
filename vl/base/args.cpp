/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-04
 */
// Interface
#include "args.hpp"

#include "base/exceptions.hpp"

vl::Args::Args( void )
	: _argv(0), _argc(1), _size(1)
{
	_argv = new char*[_size];
	_argv[0] = new char[1];
	_argv[0][0] = '\0';
}

vl::Args::Args( vl::Args const &arg )
	: _argv(0), _argc(arg._argc), _size(arg._size)
{
	this->_argv = new char*[_size];
	for( size_t i = 0; i < _argc; i++ )
	{
		_argv[i] = new char[::strlen(arg._argv[i])];
		::strcpy( _argv[i], arg._argv[i] );
	}
}

vl::Args &
vl::Args::operator=( vl::Args const &arg )
{ return *this; }

vl::Args::~Args( void )
{
	clear();
	delete [] _argv[0];
	delete [] _argv;
}

void
vl::Args::add( char const *arg )
{
	if( arg[0] == '\0' )
	{ return; }

	if( _size == _argc )
	{
		grow();
	}

	// Move the last null
	_argv[_argc] = _argv[_argc-1];

	// Copy the argument
	_argv[_argc-1] = new char[::strlen(arg)+1];
	::strcpy( _argv[_argc-1], arg );

	++_argc;
}

void
vl::Args::insert( size_t index, char const *str )
{
	if( index >= _argc )
	{
		BOOST_THROW_EXCEPTION( vl::bad_index() );
	}

	if( _argc == _size )
	{ grow(); }

	for( size_t i = _argc; i > index; --i )
	{
		_argv[i] = _argv[i-1];
	}

	_argv[index] = new char[::strlen(str)+1];
	::strcpy( _argv[index], str );
	++_argc;
}

char *
vl::Args::at( size_t index )
{
	if( index >= _argc )
	{
		BOOST_THROW_EXCEPTION( vl::bad_index() );
	}

	return _argv[index];
}

char const *
vl::Args::at( size_t index ) const
{
	if( index >= _argc )
	{
		BOOST_THROW_EXCEPTION( vl::bad_index() );
	}

	return _argv[index];
}
	
void
vl::Args::clear( void )
{
	for( size_t i = 0; i < _argc-1; ++i )
	{ delete [] _argv[i]; }

	_argv[0] = _argv[_argc-1];

	_argc = 1;
}

void
vl::Args::grow( void )
{
	_size *= 2;
	char **tmp = new char*[_size];
	for( size_t i = 0; i < _argc; ++i )
	{ tmp[i] = _argv[i]; }
	for( size_t i = _argc; i < _size; ++i )
	{ tmp[i] = 0; }

	delete [] _argv;
	_argv = tmp;
}
