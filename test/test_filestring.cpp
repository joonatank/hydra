#ifdef VL_UNIX
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MODULE filestring

#include <boost/test/unit_test.hpp>

#include "base/filesystem.hpp"

BOOST_AUTO_TEST_CASE( constructor )
{
	vl::FileString str;
	BOOST_CHECK_EQUAL( str.length, 0u );
	BOOST_CHECK_EQUAL( str.data, (char *)0 );

	std::string data( "arg . eh" );
	size_t length = data.size();
	BOOST_CHECK_EQUAL( data.c_str()[length], '\0' );
	
	vl::FileString str2( data );
	BOOST_CHECK_EQUAL( length, str2.length );
	BOOST_CHECK_EQUAL( std::string( str2.data ), data );
	BOOST_CHECK_EQUAL( str2.data[length], '\0' );
	BOOST_TEST_MESSAGE( str2.data );
}