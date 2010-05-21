#ifdef VL_UNIX
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MODULE test_args

#include <boost/test/unit_test.hpp>

#include "eq_graph/eq_root.hpp"
#include "base/exceptions.hpp"
#include "base/args.hpp"

BOOST_AUTO_TEST_CASE( add_arg )
{
	vl::Args args;
	BOOST_CHECK( args.size() == 1 );
	BOOST_CHECK( 0 == ::strcmp( args.at(0), "\0" ) );

	args.add( "test" );
	BOOST_CHECK( args.size() == 2 );
	BOOST_CHECK( 0 == ::strcmp( args.at(0), "test" ) );
	BOOST_CHECK( 0 == ::strcmp( args.at(1), "\0" ) );

	args.add( "doh" );
	BOOST_CHECK( args.size() == 3 );
	BOOST_CHECK( 0 == ::strcmp( args.at(0), "test" ) );
	BOOST_CHECK( 0 == ::strcmp( args.at(1), "doh" ) );
	BOOST_CHECK( 0 == ::strcmp( args.at(2), "\0" ) );
}

struct ArgFixture
{
	ArgFixture( void )
	{
		args.add( "test" );
		args.add( "doh" );
	}

	vl::Args args;
};

BOOST_FIXTURE_TEST_SUITE( arg_test, ArgFixture )
BOOST_AUTO_TEST_CASE( copy )
{
	vl::Args copy1(args);
	BOOST_REQUIRE_NE( args.getData(), copy1.getData() );
	BOOST_REQUIRE_EQUAL( args.size(), copy1.size());
	BOOST_CHECK( 0 == ::strcmp( args.at(0), copy1.at(0) ) );
	BOOST_CHECK( 0 == ::strcmp( args.at(1), copy1.at(1) ) );
	BOOST_CHECK( 0 == ::strcmp( args.at(2), copy1.at(2) ) );

	vl::Args copy2(args);
	BOOST_REQUIRE_NE( args.getData(), copy2.getData());
	BOOST_REQUIRE_EQUAL( args.size(), copy2.size() );
	BOOST_CHECK( 0 == ::strcmp( args.at(0), copy2.at(0) ) );
	BOOST_CHECK( 0 == ::strcmp( args.at(1), copy2.at(1) ) );
	BOOST_CHECK( 0 == ::strcmp( args.at(2), copy2.at(2) ) );

}

BOOST_AUTO_TEST_CASE( assigment )
{
	vl::Args copy3 = args;
	BOOST_REQUIRE_NE( args.getData(), copy3.getData() );
	BOOST_REQUIRE_EQUAL( args.size(), copy3.size() );
	BOOST_CHECK( 0 == ::strcmp( args.at(0), copy3.at(0) ) );
	BOOST_CHECK( 0 == ::strcmp( args.at(1), copy3.at(1) ) );
	BOOST_CHECK( 0 == ::strcmp( args.at(2), copy3.at(2) ) );
}

// We want to add another arg to different place
BOOST_AUTO_TEST_CASE( insert_arg )
{
	args.insert( 0, "gaah");

	BOOST_REQUIRE_EQUAL( args.size(), 4u );
	BOOST_CHECK_EQUAL( 0, ::strcmp( args[0], "gaah" ) );
	BOOST_CHECK_EQUAL( 0, ::strcmp( args[1], "test" ) );
	BOOST_CHECK_EQUAL( 0, ::strcmp( args[2], "doh" ) );
	BOOST_CHECK_EQUAL( 0, ::strcmp( args[3], "\0" ) );

	args.insert( 1, "huh" );
	BOOST_REQUIRE_EQUAL( args.size(), 5u );
	BOOST_CHECK_EQUAL( 0, ::strcmp( args[0], "gaah" ) );
	BOOST_CHECK_EQUAL( 0, ::strcmp( args[1], "huh" ) );
	BOOST_CHECK_EQUAL( 0, ::strcmp( args[2], "test" ) );
	BOOST_CHECK_EQUAL( 0, ::strcmp( args[3], "doh" ) );
	BOOST_CHECK_EQUAL( 0, ::strcmp( args[4], "\0" ) );
}

BOOST_AUTO_TEST_CASE( clear )
{
	BOOST_CHECK_EQUAL( args.size(), 3u );
	args.clear();
	BOOST_REQUIRE_EQUAL( args.size(), 1u );
	BOOST_CHECK_EQUAL( 0, ::strcmp( args[0], "\0" ) );

	args.add( "doh" );
	args.add( "test" );
	BOOST_REQUIRE_EQUAL( args.size(), 3u );
	BOOST_CHECK_EQUAL( 0, ::strcmp( args[0], "doh" ) );
	BOOST_CHECK_EQUAL( 0, ::strcmp( args[1], "test" ) );
	BOOST_CHECK_EQUAL( 0, ::strcmp( args[2], "\0" ) );
}

BOOST_AUTO_TEST_SUITE_END()
