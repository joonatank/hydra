#ifndef EQ_OGRE_CLIENT_HPP
#define EQ_OGRE_CLIENT_HPP

// Equalizer
#include <eq/eq.h>

// standard c++ lib
#include <memory>

//#include "sceneManager.hpp"
//#include "entity.hpp"
//#include "light.hpp"

#include "base/fifo_buffer.hpp"
//#include "server_command.hpp"
#include "eq_graph/eq_root.hpp"
//#include "udp_server.hpp"

namespace eqOgre
{
	class Config;

    class Client : public eq::Client
    {
    public:
		//Client( const InitData& initData );
		Client( void );
        virtual ~Client (void) {}

		// Create ogre::root corresponding to this client
		virtual bool initLocal( int argc, char **argv );

		virtual int run( void );

    protected:
		// eq overriden, loop invariant
        virtual bool clientLoop (void);

		// Some helper methods
		virtual int initialise( void );

		virtual void shutdown( void );

        virtual int startRendering( void );

		virtual void renderOneFrame( void );

		// Receive commands coming through network from remote driver clients.
		// These are passed further to Node (for now they are all passed to,
		// Nodes).
		// This function will throw if error occures.
//		void _receive( void );

		// Pumps messages further, that is for now it will pump all messages
		// to Node (there exists only one node per client).
//		void _pumpMessage( std::auto_ptr<vl::server::Command> cmd );
		
		eq::ServerPtr _server;

		// State the object is in, we use this to init the SceneGraph
		unsigned int _state;

//		vl::base::fifo_buffer< vl::server::Command * > *_write_node_fifo;
		eqOgre::Config *_config;

//		vl::udp::Server _udp_server;

		vl::cl::Root *_root;
    };	// class Client

}	// namespace eqOgre

#endif // EQ_OGRE_CLIENT_HPP
