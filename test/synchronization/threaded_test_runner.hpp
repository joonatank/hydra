/*	Joonatan Kuosa
 *	2010-05
 *
 *	Runner for tests that are to verify that two threads are synchronized
 *	correctly.
 */

#ifndef VL_THREADED_TEST_RUNNER_HPP
#define VL_THREADED_TEST_RUNNER_HPP

#include <deque>

#include <eq/eq.h>

namespace vl
{

// Single test to be executed on the Runner
// Test code should inherit this class and implement virtual functions
struct Test
{
	Test( bool mast )
		: master( mast )
	{}

	virtual bool test( void ) = 0;

	bool master;
};

struct TestRunner
{
	TestRunner( void ) {}

	~TestRunner( void ) {}
	
	void addTest( Test *t)
	{ tests.push_back(t); }

	bool runTest( void )
	{
		bool ret = tests.front()->test();
		tests.pop_front();
		return ret;
	}

	bool remaining( void ) const
	{ return (tests.size() != 0 ); }

	std::deque<Test *> tests;
};	// struct TestRunner

struct SyncFixture
{
	virtual ~SyncFixture( void ) {}

	virtual void init( eq::Config *conf ) = 0;
	
	virtual uint32_t reg( uint32_t id ) = 0;

	virtual void test( void ) = 0;
	
	virtual bool remaining( void ) const = 0;
};

}	// namespace vl

#endif
