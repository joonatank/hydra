
#ifndef EQ_OGRE_PIPE_H
#define EQ_OGRE_PIPE_H

#include <eq/eq.h>

#include "base/fifo_buffer.hpp"
//#include "command.hpp"
#include "base/message.hpp"

#include "eq_graph/eq_root.hpp"

/*
namespace vl
{
	namespace graph
	{
		class Root;
	}
}
*/

namespace eqOgre
{
    /**
     * The representation of one GPU.
     *
     * The pipe object is responsible for maintaining GPU-specific and
     * frame-specific data. The identifier passed by the configuration contains
     * the version of the frame data corresponding to the rendered frame. The
     * pipe's start frame callback synchronizes the thread-local instance of the
     * frame data to this version.
     */

	/*
	 * Joonatan Kuosa 2010-01
	 * Pipe is the primitive running pipe thread and it's the only one with
	 * need to access rendering engine, so it's the one owning all Ogre objects
	 * and the one responsible for managing them and also the only one thread
	 * that can access those objects
	 * (window and channel classes are run from pipe thread, so those can also
	 * access Ogre objects).
	 *
	 * Mainly responsible for running commands from command queue to update
	 * Ogre objects. And also fire rendering events to window.
	 */

    class Pipe : public eq::Pipe
    {
    public:
        Pipe( eq::Node *parent );

		vl::cl::Root *getRoot( void )
		{ return _root; }

    protected:
        virtual ~Pipe ();

        virtual bool configInit( const uint32_t initID );
        virtual bool configExit();
        virtual void frameStart( const uint32_t frameID, 
                                 const uint32_t frameNumber );

		/*
		void initOgre( void );

		void createScene( void );
		*/

		void _processCommands( void );

		vl::base::fifo_buffer<vl::base::Message *> *_read_node_fifo;
		//eqOgre::fifo_buffer<Command *> *_read_client_fifo;

		vl::cl::Root *_root;

    };	// class Pipe

}	// namespace eqOgre


#endif // EQ_OGRE_PIPE_HPP
