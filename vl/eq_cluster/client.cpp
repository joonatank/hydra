#include "client.hpp"

#include "eq_cluster/config.hpp"
//#include "server_command.hpp"

#include "interface/scene_manager.hpp"

#include "eq_graph/eq_root.hpp"


size_t const FIFO_LENGTH = 128;
unsigned int PORT = 5333;

eqOgre::Client::Client( void )
	: _state(0),
	  _config(0)
//	  _udp_server( PORT )
{} 

bool
eqOgre::Client::initLocal( int argc, char **argv )
{
	return eq::Client::initLocal( argc, argv );
}

int
eqOgre::Client::run( void )
{
	EQINFO << "Client run" << std::endl;

	int ret = initialise();
	if( ret != 0 )
	{
		EQINFO << "Client initialize failed" << std::endl;
		return ret;
	}
	ret = startRendering();
	shutdown();
	return ret;
}

// -------------------- Protected ---------------------
int 
eqOgre::Client::initialise( void )
{
	EQINFO << "Client initialise" << std::endl;

    // 1. connect to server
	// Here we fail to connect to the server so all output messages
	// are delivered to /dev/null
	_server = new eq::Server;
    if( !connectServer( _server ))
    {
        EQERROR << "Can't open server" << std::endl;
        return -1;
    }

    // 2. choose config
	EQERROR << "Client choose config" << std::endl;
	std::cout << "Client choose config" << std::endl;

    eq::ConfigParams configParams;
	eq::Config *conf =  _server->chooseConfig( configParams );
	_config = static_cast<eqOgre::Config *>( conf );

    if( !_config )
    {
        EQERROR << "No matching config on server" << std::endl;
        disconnectServer( _server );
        return -1;
    }

	/*
	std::cout << "Client creating root" << std::endl;
	_root = new vl::cl::Root();
	_config->registerObject( _root );
	eq::base::Clock clock;
	*/

//	_write_node_fifo
//		= new vl::base::fifo_buffer<vl::server::Command *>(FIFO_LENGTH);
//	_config->setClientFifo( _write_node_fifo );

    // 3. init config
	// TODO this should register data, that is the initData should be registered
	// it also should update at everyframe instead of just in the start.
	if( !_config->init( _root->getID() ) )
	{
		EQERROR << "Config initialization failed: " 
				<< _config->getErrorMessage() << std::endl;
		_server->releaseConfig( _config );
		disconnectServer( _server );
		return -1;
	}

	/*
	std::cout << "Client config inited" << std::endl;

	EQLOG( eq::LOG_STATS ) << "Config init took " << clock.getTimef() << " ms"
		<< std::endl;
	*/

	return 0;
}	// initialise

int
eqOgre::Client::startRendering( void )
{
	std::cerr << "Client::startRendering" << std::endl;

	eq::base::Clock clock;

	EQASSERT( _config );
	if( !_config->isRunning() )
	{
		std::cerr << "Config is not running" << std::endl;
	}

	// Create the scene
	
	while( _config->isRunning() )
	{
//		_receive();
		renderOneFrame();
	}

    const uint32_t frame = _config->finishAllFrames();
    const float    time  = clock.getTimef();
    EQLOG( eq::LOG_STATS ) << "Rendering took " << time << " ms (" << frame
                       << " frames @ " << ( frame / time * 1000.f) << " FPS)"
                       << std::endl;
	return 0;
}

void
eqOgre::Client::renderOneFrame( void )
{
	static uint32_t frameNumber = 0;
	EQASSERT( _config );

    if( !_config->isRunning () )
    {
		throw vl::exception( "Config is not running!",
			"Client::renderOneFrame" );
	}

	// Init the SceneGraph
	if( _state == 0 )
	{
//		EQASSERT( _write_node_fifo );

		/*
		std::cout << "Init SceneGraph" << std::endl;

		vl::graph::SceneManager *sm = _root->getSceneManager( "SceneManager" );
		vl::graph::SceneNode *node = sm->createNode( "Feet" );
		if( sm->getNode( "Feet" ) )
		{
			std::cout << "Client : Found Feet" << std::endl;
		}
		_root->commit();
		*/
//		sm->commit();
		//vl::graph::SceneNode *node = sm->createNode();
		/*	Removed the command based scene graph
		// We create the SceneManager
		vl::server::Command *cmd =
			new vl::server::CreateCmd( "SceneManager", "SceneManager" );
		_write_node_fifo->push( cmd );
		// We create the Parent Node
		cmd = new vl::server::CreateCmd( "RobotNode", "SceneNode" );
		_write_node_fifo->push( cmd );
		// Create Entity and attach it to node we just created
		vl::NamedValuePairList param;
		param["mesh"] = "robot.mesh";
		cmd = new vl::server::CreateCmd( "Robot", "Entity", "RobotNode", param );
		_write_node_fifo->push( cmd );

		// Move the RobotNode
		cmd = new vl::server::MoveCmd( "RobotNode", "SceneNode",
				vmml::vec3d( -75, 0, 500 ) );
		_write_node_fifo->push( cmd );
		*/
			
		++_state;
	}
	// Move an object
	else if( _state > 0 )
	{
		/*
		EQASSERT( _write_node_fifo );

		static bool right = false;

		// Move a node
		vmml::vec3d vec;
		// We are going to the left
		if( frameNumber % 1000 < 500)
		{ right = true; }
		// We are going to the right
		else if( frameNumber % 1000 > 500)
		{ right = false; }

		if( !right )
		{ vec = vmml::vec3d( -0.3, 0, 0 ); }
		else
		{ vec = vmml::vec3d( 0.3, 0, 0 ); }

		vl::server::Command *cmd =
			new vl::server::MoveCmd( "RobotNode", "SceneNode", vec );
		_write_node_fifo->push( cmd );
		*/
	}

	_config->startFrame( ++frameNumber );

	_config->finishFrame();

	/*
	while( !_config->needsRedraw() ) // wait for an event requiring redraw
	{
		if( hasCommands() ) // execute non-critical pending commands
		{
			processCommand();
			_config->handleEvents(); // non-blocking
		}
		else  // no pending commands, block on user event
		{
			const eq::ConfigEvent* event = _config->nextEvent();
			if( !_config->handleEvent( event ) )
			{ EQVERB << "Unhandled " << event << endl; }
		}
	}
	*/
	_config->handleEvents (); // process all pending events
}

void
eqOgre::Client::shutdown( void )
{
//	std::cout << "Client shutting down" << std::endl;

	// 5. exit config
	EQASSERT( _config );
	_config->exit();

    // 6. cleanup and exit
	_server->releaseConfig( _config );
	if ( !disconnectServer( _server ) )
	{ EQERROR << "Client::disconnectServer failed" << std::endl; }

    _server = 0;
}

/*
void
eqOgre::Client::_receive( void )
{
	// Receive udp messages and pump them through message control
	_udp_server.receive();
	std::auto_ptr<vl::server::Command> cmd = _udp_server.popCommand();
	while( cmd.get() )
	{
		_pumpMessage( cmd );
		cmd = _udp_server.popCommand();
	}
}

void
eqOgre::Client::_pumpMessage( std::auto_ptr<vl::server::Command> cmd )
{
	if( !cmd.get() )
	{ return; }
	_write_node_fifo->push( cmd.release() );
}
*/

bool
eqOgre::Client::clientLoop( void )
{
	return eq::Client::clientLoop();
	/*
	// TODO add initialized variable
	if( !_initData.isResident( ) ) // execute only one config run
	{ return eq::Client::clientLoop(); }

    // else execute client loops 'forever'
    while ( true )
    {
		if ( !eq::Client::clientLoop () )
		{ return false; }
		EQINFO << "One configuration run successfully executed" << endl;
    }
    return true;
	*/
}
