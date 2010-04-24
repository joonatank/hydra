/*	Joonatan Kuosa
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
#ifndef VL_TEST_ARGS_HPP
#define VL_TEST_ARGS_HPP

#include <ostream>

struct Args
{
	Args( void )
		: argv(0), argc(1), size(1)
	{
		argv = new char*[size];
		argv[0] = new char[1];
		argv[0][0] = '\0';
	}

	~Args( void )
	{
		for( int i = 0; i < size; i++ )
		{
			delete [] argv[i];
		}
		delete [] argv;
	}

	void addArg( char const *arg )
	{
		if( arg[0] == '\0' )
		{ return; }
	
		if( size == argc )
		{
			grow();
		}

		// Move the last null
		argv[argc] = argv[argc-1];

		// Copy the argument
		argv[argc-1] = new char[strlen(arg)+1];
		strcpy( argv[argc-1], arg );

		++argc;
	}

	void grow( void )
	{
		size *= 2;
		char **tmp = new char*[size];
		for( int i = 0; i < argc; ++i )
		{ tmp[i] = argv[i]; }

		delete [] argv;
		argv = tmp;
	}

	char **argv;
	int argc;
	int size;
};

std::ostream &operator<<( std::ostream &os, ::Args const &arg )
{
	for( int i = 0; i < arg.argc; ++i )
	{ os << arg.argv[i] << ' '; }

	return os;
}

#endif
