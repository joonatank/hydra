/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
 *
 */


#ifndef VL_TEST_CLIENT_FIXTURES_HPP
#define VL_TEST_CLIENT_FIXTURES_HPP

// eqOgre Library includes
#include "eq_cluster/client.hpp"
#include "eq_cluster/config.hpp"
#include "eq_cluster/window.hpp"
#include "eq_cluster/channel.hpp"
#include "eq_cluster/pipe.hpp"
#include "base/system_util.hpp"
#include "eq_ogre/ogre_root.hpp"
#include "settings.hpp"


// Test helpers
#include "../debug.hpp"

struct ListeningClientFixture
{
	ListeningClientFixture( void ) {}

	~ListeningClientFixture( void )
	{ exit(); }

	bool init( vl::SettingsRefPtr settings,
			   eq::NodeFactory *nodeFactory )
	{
		InitFixture();
		std::string where( "ListeningClientFixture::init" );

		if( !settings )
		{
			std::cerr << where << " : settings are invalid." << std::endl;
			return false;
		}
		if( !nodeFactory )
		{
			std::cerr << where << " : nodeFactory is invalid." << std::endl;
			return false;
		}

		// TODO test if this can work with relative path
		// Oh right it doesn't for autolaunched clients.
		// NOTE the log file needs to be set before any calls to Equalizer methods
		log_file.open( settings->getEqLogFilePath().c_str() );
		eq::base::Log::setOutput( log_file );
		std::cout << "Equalizer log file = " << settings->getEqLogFilePath()
			<< std::endl;

		vl::Args &arg = settings->getEqArgs();

		std::cout << "Args = " << settings->getEqArgs() << std::endl;
		// 1. Equalizer initialization
		if( !eq::init( arg.size(), arg.getData(), nodeFactory ) )
		{
			EQERROR << "Equalizer init failed" << std::endl;
			return false;
		}

		// 2. initialization of local client node
		client = new eqOgre::Client( settings );
		if( !client->initLocal( arg.size(), arg.getData() ) )
		{
			EQERROR << "client->initLocal failed" << std::endl;
			return false;
		}

		if( !client->initialise() )
		{
			EQERROR << "client->init failed" << std::endl;
			return false;
		}

		return true;
	}

	bool mainloop( const uint32_t frame )
	{
		return client->mainloop( frame );
	}

	void exit( void )
	{
		client = 0;

		eq::exit();
	}

	std::ofstream log_file;

	eq::base::RefPtr< eqOgre::Client > client;
};

#endif