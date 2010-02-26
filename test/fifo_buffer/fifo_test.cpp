#include <iostream>
#include <cstring>
#include <cstdlib>
#include <sys/time.h>

#include "../fifo_buffer.hpp"

#include "fifo_test.hpp"
sig_atomic_t g_timer_off = 0;

void *
write_data_ptr( void *ptr )
{
	if( !ptr )
	{ return (void *)1; }

	eqOgre::fifo_buffer<char *> *fifo = (eqOgre::fifo_buffer<char *> *)(ptr);

	while( g_timer_off < RUNTIME_SEC )
	{
		for( unsigned int i = 0; i < 5; i++ )
		{
			char *str = new char[64];
			static size_t count = 0;
			strcpy( str, STRINGS[i] );

			fifo->push( str );
		}
		
		// Sleep for a while
		struct timespec tv;
		tv.tv_sec = 0;
		tv.tv_nsec = 1000*1000;
		nanosleep( &tv, 0 );
	}

	return 0;
}

void *
read_data_ptr( void *ptr )
{
	std::cout << "Entered read_data_ptr thread" << std::endl;

	if( !ptr )
	{ return (void *)1; }

	eqOgre::fifo_buffer<char *> *fifo = (eqOgre::fifo_buffer<char *> *)(ptr);
	
	size_t count = 0;
	size_t secs = 0;
//	bool overflow = false;
	while( g_timer_off < RUNTIME_SEC )
	{
		char* str = fifo->pop();
		if( str )
		{
//			if( count + 1 < count )
//			{ overflow = true; }
			count++;
			//std::cout << str << std::endl;
			delete str;
			str = 0;
		}
		if( g_timer_off > secs )
		{
			secs = g_timer_off;
//			if( overflow )
//			{ std::cout << "Overflown" << std::endl; }
			std::cout << "Read " << count << " lines in " << secs
				<< " seconds." << std::endl;
		}
	}

	return 0;
}

void *
write_data( void *ptr )
{
	if( !ptr )
	{ return (void *)1; }

	eqOgre::fifo_buffer<Test> *fifo = (eqOgre::fifo_buffer<Test> *)(ptr);

	while( g_timer_off < RUNTIME_SEC )
	{
		for( unsigned int i = 0; i < 17; i++ )
		{ fifo->push( Test( TEST_STRING[i] ) ); }
		
		// Sleep for a while
		struct timespec tv;
		tv.tv_sec = 0;
		tv.tv_nsec = 1000*1000;
		nanosleep( &tv, 0 );
	}

	return 0;
}

void *
read_data( void *ptr )
{
	std::cout << "Entered read_data_ptr thread" << std::endl;

	if( !ptr )
	{ return (void *)1; }

	eqOgre::fifo_buffer<Test> *fifo = (eqOgre::fifo_buffer<Test> *)(ptr);
	
	size_t count = 0;
	size_t secs = 0;
	while( g_timer_off < RUNTIME_SEC )
	{
		Test ch = fifo->pop();
		if( ch != Test() )
		{
			count++;
		}
		if( g_timer_off > secs )
		{
			secs = g_timer_off;
			std::cout << "Read " << count << " times in " << secs
				<< " seconds." << std::endl;
		}
	}

	return 0;
}

void *
read_data_cout( void *ptr )
{
	if( !ptr )
	{ return (void *)1; }

	eqOgre::fifo_buffer<Test> *fifo = (eqOgre::fifo_buffer<Test> *)(ptr);
	
	size_t count = 0;
	size_t secs = 0;
	char buffer[2048];
	while( g_timer_off < RUNTIME_SEC )
	{
		Test ch = fifo->pop();
		if( ch != Test() )
		{
			buffer[count] = ch._ch;
			count++;
		}
		
		if( count == 2048 )
		{
			std::cout << "buffer = " << buffer;
			count = 0;
		}
		/*
		if( g_timer_off > secs )
		{
			secs = g_timer_off;
			std::cout << "Read " << count << " times in " << secs
				<< " seconds." << std::endl;
		}
		*/
	}

	return 0;
}
