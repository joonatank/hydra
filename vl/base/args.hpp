/**	Joonatan Kuosa
 *	2010-04
 *
 * 	class Args
 *	Fake arguments passed to the application.
 *
 *	Utility class for Unit testing where the test runner does not allow
 *	passing arguments to specific test cases.
 *
 *	Also useful for testing application command line parsers.
 */
#ifndef VL_FAKE_ARGS_HPP
#define VL_FAKE_ARGS_HPP

#include <ostream>
#include <cstring>

namespace vl
{

class Args
{
public :
	Args( void );

	Args( Args const &arg );

	Args &operator=( Args const &arg );

	~Args( void );

	void add( char const *arg );

	void insert( size_t index, char const *str );

	char *operator[ ]( size_t index )
	{ return _argv[index]; }

	char const *operator[ ]( size_t index ) const
	{ return _argv[index]; }

	char *at( size_t index );

	char const *at( size_t index ) const;

	size_t &size( void )
	{ return _argc; }

	bool empty( void ) const
	{ return 0 == _argc; }

	char **getData( void )
	{ return _argv; }

	void clear( void );

	friend std::ostream &operator<<( std::ostream &os, Args const &arg );

private :
	void grow( void );

	char **_argv;
	size_t _argc;
	size_t _size;
};

	std::ostream &operator<<( std::ostream &os, Args const &arg );
}	// namespace vl

inline std::ostream &
vl::operator<<( std::ostream &os, vl::Args const &arg )
{
	for( size_t i = 0; i < arg._argc; ++i )
	{ os << arg._argv[i] << ' '; }

	return os;
}

#endif
