#ifdef VL_UNIX
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MODULE settings

#include <boost/test/unit_test.hpp>

#include "settings.hpp"

BOOST_AUTO_TEST_CASE( constructor )
{
	BOOST_CHECK_NO_THROW( vl::Settings() )
}

BOOST_AUTO_TEST_CASE( read_from_file )
{

}

BOOST_AUTO_TEST_CASE( write_to_file )
{

}
