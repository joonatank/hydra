#ifdef VL_UNIX
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MODULE exceptions

#include <boost/test/unit_test.hpp>

#include "base/exceptions.hpp"

#include <iostream>

BOOST_AUTO_TEST_CASE( exceptions )
{
	try {
		BOOST_THROW_EXCEPTION( vl::null_pointer() );
	}
	catch( std::exception const &e )
	{
		std::cerr << "Catched std::exception : " << e.what() << std::endl;
	}

	try
	{
		BOOST_THROW_EXCEPTION( vl::null_pointer() );
	}
	catch( boost::exception const &e )
	{
		std::cerr << "Catched boost::exception : "
			<< boost::diagnostic_information<>(e) << std::endl;
	}
}