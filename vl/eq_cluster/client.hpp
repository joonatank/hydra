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
		Client( vl::SettingsRefPtr settings );

        virtual ~Client (void);

		int run( void );

	protected:

		/// Equalizer overrides
		// Needs to be overriden if we want that the listening client stays
		// running forever and not quit when the application quits.
//		virtual void clientLoop (void);

		eq::ServerPtr _server;
		eqOgre::Config *_config;
		vl::SettingsRefPtr _settings;

    };	// class Client

}	// namespace eqOgre

#endif // EQ_OGRE_CLIENT_HPP
