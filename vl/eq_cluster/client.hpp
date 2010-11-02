#ifndef EQ_OGRE_CLIENT_HPP
#define EQ_OGRE_CLIENT_HPP

// Equalizer
#include <eq/eq.h>

#include "base/typedefs.hpp"

namespace eqOgre
{
	class Config;

    class Client : public eq::Client
    {
    public:
		Client( eqOgre::SettingsRefPtr settings );

        virtual ~Client (void);

		// Create ogre::root corresponding to this client
		virtual bool initLocal( int const argc, char **argv );

		virtual bool mainloop( uint32_t frame );

		// Some helper methods
		virtual bool initialise( void );

	protected:
		
		virtual void exit( void );

		// eq overriden, loop invariant
        virtual void clientLoop (void);
		
		eq::ServerPtr _server;
		eqOgre::Config *_config;
		eqOgre::SettingsRefPtr _settings;

    };	// class Client

}	// namespace eqOgre

#endif // EQ_OGRE_CLIENT_HPP
