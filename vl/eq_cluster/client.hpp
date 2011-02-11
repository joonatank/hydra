/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2010-12
 *
 *	Program starting point
 *	TODO should probably be replaced with just main function
 */
#ifndef EQ_OGRE_CLIENT_HPP
#define EQ_OGRE_CLIENT_HPP

#include <OGRE/OgreTimer.h>

#include <boost/thread.hpp>

#include <stdint.h>

#include "typedefs.hpp"

namespace eqOgre
{
	class Config;
	class Pipe;

    class Client
    {
    public:
		Client( vl::EnvSettingsRefPtr env, vl::Settings const &settings );

        virtual ~Client (void);

		bool run( void );

		void init( void );

	protected:

		void _exit( void );

		void _render( uint32_t const frame );

		/// Resources
		void _createResourceManager( vl::Settings const &settings, vl::EnvSettingsRefPtr env );


		vl::EnvSettingsRefPtr _env;

		vl::GameManagerPtr _game_manager;

		// Clocks
		Ogre::Timer _clock;
		// Used to measure one frame time
		Ogre::Timer _frame_clock;
		// Time used for rendering the frames
		// (well not excatly as there is lots of other stuff in there also)
		// but this does not have the sleeping time
		double _rendering_time;

		eqOgre::Config *_config;

		boost::thread *_pipe_thread;
		eqOgre::Pipe *_pipe;

    };	// class Client

}	// namespace eqOgre

#endif // EQ_OGRE_CLIENT_HPP
