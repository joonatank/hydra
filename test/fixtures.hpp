#ifndef VL_TEST_FIXTURES_HPP
#define VL_TEST_FIXTURES_HPP

#include <boost/test/unit_test.hpp>

namespace test = boost::unit_test::framework;

struct InitFixture
{
	InitFixture( void )
	{
#ifdef VL_WIN32 
#ifdef _DEBUG
		_CrtSetDbgFlag( 0 );
		_CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_WNDW );
#endif	// _DEBUG
#endif	// _VL_WIN32
	}

	~InitFixture( void )
	{}
};

#endif