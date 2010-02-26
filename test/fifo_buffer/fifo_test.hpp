#ifndef FIFO_TEST_HPP
#define FIFO_TEST_HPP

#include <signal.h>

int const RUNTIME_SEC = 10;

extern sig_atomic_t g_timer_off;

class Test
{
	public :
		Test( char ch = 0 )
			: _ch( ch )
		{}
	
		char _ch;
};

inline bool
operator==( Test const &t1, Test const &t2 )
{ return t1._ch == t2._ch; }

inline bool
operator!=( Test const &t1, Test const &t2 )
{ return t1._ch != t2._ch; }

char const * const STRINGS[5] =
{
"Man this is dull\n",
"Man\n",
 "dull\n",
"whoah\n",
"dog\n"
};

char const * const TEST_STRING = "Man this is dull\n";

// Thread functions
extern "C" void *
write_data_ptr( void *ptr );

extern "C" void *
read_data_ptr( void *ptr );

extern "C" void *
write_data( void *ptr );

extern "C" void *
read_data( void *ptr );

extern "C" void *
read_data_cout( void *ptr );

#endif
