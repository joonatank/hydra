#include "client.hpp"

#include "eq_cluster/config.hpp"

eqOgre::Client::Client( eqOgre::InitData const &data )
	: _config(0), _init_data( data )
{} 

eqOgre::Client::~Client(void )
{
	exit();
}

bool
eqOgre::Client::initLocal( int argc, char **argv )
{
	return eq::Client::initLocal( argc, argv );
}

bool 
eqOgre::Client::initialise( void )
{
	std::cout << "creating server" << std::endl;
	// 1. connect to server
	_server = new eq::Server;

	if( !connectServer( _server ))
	{
		EQERROR << "Can't open server" << std::endl;
		return false;
	}

	// 2. choose config
	eq::ConfigParams configParams;
	_config = static_cast<eqOgre::Config*>(_server->chooseConfig( configParams ));

	if( !_config )
	{
		EQERROR << "No matching config on server" << std::endl;
		disconnectServer( _server );
		return false;
	}

	// 3. init config
	_config->setInitData( _init_data );

	if( !_config->init(0) )
	{
		EQERROR << "Config initialization failed: "
				<< _config->getErrorMessage() << std::endl;
		_server->releaseConfig( _config );
		disconnectServer( _server );
		return false;
	}

	return true;

}	// initialise

bool 
eqOgre::Client::mainloop( uint32_t frame )
{
	if( _config->isRunning() )
	{
		_config->startFrame( frame );
		_config->finishFrame();
		return true;
	}
	else
	{ return false; }
}

// -------------------- Protected ---------------------
void
eqOgre::Client::exit( void )
{
	if( _config )
	{
		_config->exit();
		if( _server.isValid() )
		{
			_server->releaseConfig( _config );
		}
		_config = 0;
	}
	if( _server.isValid() )
	{
		if( !disconnectServer( _server ))
			EQERROR << "Client::disconnectServer failed" << std::endl;
		_server = 0;
	}
}

bool 
eqOgre::Client::clientLoop( void )
{
	if( !isLocal() ) // execute only one config run
	{ return eq::Client::clientLoop(); }

	// else execute client loops 'forever'
	while( true ) // TODO: implement SIGHUP handler to exit?
	{
		if( !eq::Client::clientLoop( ))
			return false;
		EQINFO << "One configuration run successfully executed" << std::endl;
	}
	
	return true;
}
