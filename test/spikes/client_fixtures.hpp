#ifndef VL_TEST_CLIENT_FIXTURES_HPP
#define VL_TEST_CLIENT_FIXTURES_HPP

// eqOgre Library includes
#include "eq_cluster/client.hpp"
#include "eq_cluster/config.hpp"
#include "eq_cluster/window.hpp"
#include "eq_cluster/channel.hpp"
#include "eq_cluster/init_data.hpp"
#include "base/helpers.hpp"
#include "eq_ogre/ogre_root.hpp"
#include "settings.hpp"


// Test helpers
#include "../debug.hpp"

class NodeFactory : public eq::NodeFactory
{
public:
	virtual eq::Window *createWindow( eq::Pipe *parent )
	{ return new eqOgre::Window( parent ); }

	virtual eq::Channel *createChannel( eq::Window *parent )
	{ return new eqOgre::Channel( parent ); }

	virtual eq::Config *createConfig( eq::ServerPtr parent )
	{ return new eqOgre::Config( parent ); }
};

struct ListeningClientFixture
{
	ListeningClientFixture( void ) {}

	~ListeningClientFixture( void )
	{ exit(); }

	bool init( eqOgre::InitData &initData,
			   eq::NodeFactory *nodeFactory )
			   //std::string const &project_name )
	{
		InitFixture();

		vl::SettingsRefPtr settings = initData.getSettings();

		// Create eq log file
		uint32_t pid = vl::getPid();
		std::stringstream ss;
		if( !settings->getLogDir().empty() )
		{ ss << settings->getLogDir() << "/"; }

		ss << initData.getName() << "_eq_" << pid << ".log";
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
		client = new eqOgre::Client( initData );
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