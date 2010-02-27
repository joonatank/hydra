#include "config.hpp"
#include "eq_ogre/ogre_root.hpp"
#include "channel.hpp"

#include <OgreDefaultHardwareBufferManager.h>

#include "node.hpp"

const size_t FIFO_LENGTH = 128;

eqOgre::Node::Node (eq::Config* parent )
	: eq::Node( parent),
//	  _read_client_fifo(0),
	  _write_pipe_fifo(0),
	  _root(0),
	  _sm(0),
	  _state(0)
{}

eqOgre::Node::~Node( void )
{
//	delete _root;
}

bool
eqOgre::Node::configInit( const uint32_t initID )
{
	EQINFO << "Node::configInit" << std::endl;

	if( !eq::Node::configInit( initID ))
	{ return false; }

	// SceneGraph is not multi-buffered so we need to use DRAW_SYNC
	// Though it should not make difference (as we don't check for it and will
	// always sync the Pipe threads to Node thread).
	if( getIAttribute( IATTR_THREAD_MODEL ) == eq::UNDEFINED )
	{ setIAttribute( IATTR_THREAD_MODEL, eq::DRAW_SYNC ); }

	// Create ogre root
	initOgre( initID );
	EQASSERT( _root );

	// We init Ogre on all Pipe threads...
	// um we can not as Ogre::Root is singleton
	// So more than one PipeThread is going to mess with things.
	// FIXME we need to provide version which does use shared Singletons for
	// more than one PipeThread... also this version has to keep sure that it
	// only reads from managers. Argh...
	//
	// Similar problem comes with resources, which should be shared among Pipes
	// but should be created from NodeThread as only one NodeThread can exist
	// in one process.

	eqOgre::Config *config = (eqOgre::Config *)getConfig();

	// Create a command buffer to pass commands to Pipe
	_write_pipe_fifo =
		new vl::base::fifo_buffer<vl::base::Message *>(FIFO_LENGTH);
	config->setNodeFifo( _write_pipe_fifo );

	// Get fifo buffer for reading commands from client (or mainloop)
//	_read_client_fifo = config->getClientFifo();
//	EQASSERT( _read_client_fifo );
	// We test that we haven't passed wrong pointer to setFifo()
//	EQASSERT( (void *)_read_client_fifo  != (void *)_write_pipe_fifo );

    return true;
}

bool
eqOgre::Node::configExit()
{
//	eqOgre::Config* config = static_cast< eqOgre::Config* >( getConfig( ));
 //   config->unmapData();
    
    return eq::Node::configExit();
}

void
eqOgre::Node::frameStart(const uint32_t frameID, const uint32_t frameNumber )
{
	if( _state < 1 )
	{
		if( _root )
		{
			//std::cout
			EQERROR << "Root sync" << std::endl;
			_root->sync();
			vl::graph::SceneManager *sm = _root->getSceneManager();
			//std::cout << "SceneManager sync" << std::endl;
			//sm->sync();
			++_state;

			if( sm )
			{
				std::cout << "SceneManager present" << std::endl;
				vl::graph::SceneNode *node = sm->getNode( std::string("Feet") );
				if( node )
				{
					std::cout << "Feet found from scene graph" << std::endl;
					//++_state;
				}
				else
				{
					std::cout << "Feet NOT found" << std::endl;
				}
			}
		}
		// Pass pointer to Ogre Root to Pipe Thread, so it can be used to create
		// windows, viewports and cameras
		// TODO we should really create those windows, viewports and cameras
		// here (after frameStart) and pass pointer to those instead of passing
		// the Root object.
		// TODO we should also not share native pointers, but the abstract ones
		// FIXME we need this for window creation, but we don't want to pass
		// Ogre::Root through the fifo
		// This would work now, but we don't want to use it.
		// Rather pass Window and Camera containers through this.
		// Copy the stuff from window.cpp camera creation.
		//	vl::base::Message *msg
		//	= new vl::base::RootTransfer( _root->getNative() );
		//	_write_pipe_fifo->push( msg );
	}

	// TODO add process commands
	// Which will modify the SceneGraph

	// Last command we call will release the Node lock on the SceneGraph
	// and let Pipes start rendering
	_processCommands();

	eq::Node::frameStart( frameID, frameNumber );
}

// -------------------- Protected --------------------
void
eqOgre::Node::initOgre( uint32_t initID )
{
	EQINFO << "Pipe::initOgre" << std::endl;

	// Override the default logging to provide only file logging,
	// we have tree logs at the moment so we really don't want it all to flood
	// out std::cerr...
	// TODO move to abstract methods
	Ogre::LogManager *log_man = new Ogre::LogManager();
	log_man->createLog( "ogre.log", true, false );

	std::cout << "Node : Mapping root" << std::endl;
	_root = new vl::ogre::Root();
	getSession()->mapObject( _root, initID );
	// TODO add the necessary functions to vl::ogre::Root to initialize
	// Ogre::Root
	_root->createRenderingSystem();
	_root->init();
}

/*
void
eqOgre::Node::createScene( void )
{
	EQINFO << "Creating scene." << std::endl;

	// TODO
	// Some crappy test code, please remove as soon as tested that node
	// creations and rendering works.
	EQASSERT( _sm );

	eqOgre::Entity *ent = _sm->createEntity( "Robot", "robot.mesh" );
	eqOgre::SceneNode *node = _sm->getRootSceneNode()
			->createChildSceneNode( "RobotNode", Ogre::Vector3(0, 0, 500) );
	node->attachObject( ent );
}
*/

void
eqOgre::Node::_processCommands( void )
{
	/*
	EQASSERT( _read_client_fifo );

	vl::server::Command *cmd = 0;
	while( (cmd = _read_client_fifo->pop()) )
	{
		// FIXME commands are not used at the moment... because of structural
		// changes they should be made to work with vl::graph objects
		// not Ogre or vl::ogre objects
	//	cmd->operator()( _root );

		delete cmd;

//		eqOgre::SceneManager *sm = 0;

	}
	*/
}
