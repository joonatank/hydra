#ifdef VL_UNIX
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MODULE ogre_root

#include <boost/test/unit_test.hpp>

#include "eq_graph/eq_root.hpp"
#include "base/exceptions.hpp"

#include "args.hpp"

BOOST_AUTO_TEST_CASE( args_test )
{
	::Args args;
	BOOST_CHECK( args.size == 1 );
	BOOST_CHECK( args.argc == 1 );
	BOOST_CHECK( 0 == ::strcmp( args.argv[0], "\0" ) );

	args.addArg( "test" );
	BOOST_CHECK( args.size == 2 );
	BOOST_CHECK( args.argc == 2 );
	BOOST_CHECK( 0 == ::strcmp( args.argv[0], "test" ) );
	BOOST_CHECK( 0 == ::strcmp( args.argv[1], "\0" ) );

	args.addArg( "doh" );
	BOOST_CHECK( args.size == 4 );
	BOOST_CHECK( args.argc == 3 );
	BOOST_CHECK( 0 == ::strcmp( args.argv[0], "test" ) );
	BOOST_CHECK( 0 == ::strcmp( args.argv[1], "doh" ) );
	BOOST_CHECK( 0 == ::strcmp( args.argv[2], "\0" ) );
}
