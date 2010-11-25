
#ifndef EQOGRE_PIPE_HPP
#define EQOGRE_PIPE_HPP

#include <eq/eq.h>

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

		// NOTE this causes the a freeze in Windows.
		// TODO Solve the message pumping problem
		// Can we use the Equalizer message pump?
		// Or do we have to create one to do Windows message pumping?

		// Equalizer MessagePump seems to work on Windows
		// TODO test on Linux
//		virtual eq::MessagePump *createMessagePump() { return 0; }

    protected:
		/*	These are completely useless
        virtual ~Pipe ();

        virtual bool configInit( const uint32_t initID );
        virtual bool configExit();
        virtual void frameStart( const uint32_t frameID, 
                                 const uint32_t frameNumber );
		*/

		// This seems not to be needed
		// At least on Windows
		// On Linux the newer Equalizer version should have removed 
		// the event handling from eq::GLXPipe and it's in eq::GLXWindow
		// similar to Windows'.
//		virtual bool configInitSystemPipe( const uint32_t );

    };	// class Pipe

}	// namespace eqOgre


#endif // EQ_OGRE_PIPE_HPP
