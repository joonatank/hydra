#ifndef SYNC_FIXTURE_HPP
#define SYNC_FIXTURE_HPP

#include <eq/eq.h>

struct SyncFixture
{
	SyncFixture( eq::Config *conf, int n_tests )
		: config( conf ), N_TESTS(n_tests), state(0)
	{}

	virtual ~SyncFixture( void )
	{}

	virtual uint32_t init( uint32_t id = EQ_ID_INVALID ) = 0;

	virtual void update( void ) = 0;

	virtual void test( int state ) = 0;

	bool testRemaining( void )
	{
		return state < N_TESTS-1;
	}

	int numTests( void )
	{
		return N_TESTS;
	}

	eq::Config *config;
	int N_TESTS;
	int state;
};

#endif
