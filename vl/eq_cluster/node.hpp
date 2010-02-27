
#ifndef EQ_OGRE_NODE_HPP
#define EQ_OGRE_NODE_HPP

#include <eq/eq.h>

#include "base/message.hpp"

// Distributed root
#include "eq_graph/eq_root.hpp"
#include "eq_graph/eq_scene_manager.hpp"

namespace eqOgre
{
    /**
     * Representation of a node in the cluster
     * 
     * Manages node-specific data, namely requesting the mapping of the
     * initialization data by the local Config instance.
     */

	/*
	 * Joonatan Kuosa 2010-01
	 *
	 * Node represents rendering client
	 * It is the only object allowed to modify the SceneGraph.
	 * We can command it using UDP packets which are handled by _processCommands
	 * method, at every frame.
	 *
	 * Node drives the rendering by issuing rendering commands to Pipe Threads.
	 *
	 * Node passes Pipe specific commands to Pipes. So Node is the only object
	 * application programmers will have to worry about.
	 */

    class Node : public eq::Node
    {
    public:
        Node (eq::Config* parent );

    protected:
        virtual ~Node( void );

        virtual bool configInit( const uint32_t initID );
        virtual bool configExit();
		virtual void frameStart(const uint32_t frameID,
				const uint32_t frameNumber );

		virtual void _processCommands( void );

		void initOgre( uint32_t initID );

		// TODO this should be UDP command buffer
		// for now we use client to issue commands.
//		vl::base::fifo_buffer<vl::server::Command *> *_read_client_fifo;

		vl::base::fifo_buffer<vl::base::Message *> *_write_pipe_fifo;

		vl::cl::Root *_root;

		// SceneManager synced from Client
		vl::cl::SceneManager *_sm;

		int _state;

    };	// class Node

}

#endif // EQ_OGRE_NODE_HPP
