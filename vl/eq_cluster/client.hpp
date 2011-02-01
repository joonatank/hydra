/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-12
 *
 */
#ifndef EQ_OGRE_CLIENT_HPP
#define EQ_OGRE_CLIENT_HPP

// Equalizer
#include <eq/eq.h>

#include "typedefs.hpp"

namespace eqOgre
{
	class Config;

    class Client : public eq::Client
    {
    public:
		Client( vl::SettingsRefPtr settings );

        virtual ~Client (void);

		bool run( void );

	protected:

		bool _init( eq::Config *config );
		void _exit( void );

		void _render( uint32_t const frame );

		/// Resources
		void _createResourceManager( void );

		// Clocks
		eq::base::Clock _clock;
		// Used to measure one frame time
		eq::base::Clock _frame_clock;
		// Time used for rendering the frames
		// (well not excatly as there is lots of other stuff in there also)
		// but this does not have the sleeping time
		double _rendering_time;

		vl::SettingsRefPtr _settings;

		vl::GameManagerPtr _game_manager;

		eq::ServerPtr _server;
		eqOgre::Config *_config;

		/// Equalizer overrides
		// Needs to be overriden if we want that the listening client stays
		// running forever and not quit when the application quits.
//		virtual void clientLoop (void);

    };	// class Client

}	// namespace eqOgre

#endif // EQ_OGRE_CLIENT_HPP
