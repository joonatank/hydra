#ifndef VL_TEST_FIXTURES_HPP
#define VL_TEST_FIXTURES_HPP

#include <boost/test/unit_test.hpp>

#include "base/typedefs.hpp"
#include "settings.hpp"

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

vl::SettingsRefPtr getSettings( char *exe_path )
{
	fs::path cmd( exe_path );

	// Lets find in which directory the test_conf.xml is
	fs::path conf_dir = cmd.parent_path();
	fs::path conf = conf_dir / "test_conf.xml";
	if( !fs::exists( conf ) )
	{
		return vl::SettingsRefPtr();
	}

	vl::SettingsRefPtr settings( new vl::Settings );
	vl::SettingsSerializer ser(settings);
	ser.readFile( conf.file_string() );

	settings->setExePath( exe_path );

	return settings;
}

#endif