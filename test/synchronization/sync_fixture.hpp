#ifndef VL_SYNC_FIXTURE_HPP
#define VL_SYNC_FIXTURE_HPP

#include <eq/eq.h>

#include "threaded_test_runner.hpp"

namespace vl
{
struct SyncFixture
{
	virtual ~SyncFixture( void )
	{
	}

	virtual void init( eq::Config *conf ) = 0;
	
	virtual uint32_t reg( uint32_t id ) = 0;

	virtual void test( void ) = 0;
	
	virtual bool remaining( void ) const = 0;
};

struct EqSyncFixture : public vl::SyncFixture
{
	EqSyncFixture( bool m, eq::Object *obj )
		: master(m),
		  object(obj),
		  config(0),
		  runner()
	{}

	~EqSyncFixture( void )
	{
		/*
		if( master )
		{
			config->deregisterObject( object );
		}
		else
		{
			config->unmapObject( object );
		}
		*/
	}

	virtual void init( eq::Config *conf ) 
	{ config = conf; }
	
	virtual uint32_t reg( uint32_t id )
	{
		if( master )
		{
			BOOST_REQUIRE( config->registerObject( object ) );
			BOOST_REQUIRE( object->getID() != EQ_ID_INVALID );
			return object->getID();
		}
		else
		{
			BOOST_REQUIRE( id != EQ_ID_INVALID );
			BOOST_REQUIRE( config );
			BOOST_REQUIRE( config->mapObject( object, id ) );
			return object->getID();
		}
	}

	virtual void test( void )
	{ runner.runTest(); }

	virtual bool remaining( void ) const
	{ return runner.remaining(); }

	bool master;
	eq::Object *object;
	eq::Config *config;
	vl::TestRunner runner;
};

}	// namespace vl

#endif
