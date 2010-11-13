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

	bool init( eqOgre::SettingsRefPtr settings,
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

		// Create eq log file
		// TODO this should be cleaned and moved to eqOgre::Client
		uint32_t pid = vl::getPid();
		std::stringstream ss;
		if( !settings->getLogDir().empty() )
		{ ss << settings->getLogDir() << "/"; }

		ss << settings->getProjectName() << "_eq_" << pid << ".log";
		log_file.open( ss.str().c_str() );

		eq::base::Log::setOutput( log_file );

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