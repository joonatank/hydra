#ifndef EQ_OGRE_CLIENT_HPP
#define EQ_OGRE_CLIENT_HPP

// Equalizer
#include <eq/eq.h>

#include "init_data.hpp"

namespace eqOgre
{
	class Config;

    class Client : public eq::Client
    {
    public:
		//Client( const InitData& initData );
		Client( eqOgre::InitData const &data );
        virtual ~Client (void) {}

		// Create ogre::root corresponding to this client
		virtual bool initLocal( int argc, char **argv );

		virtual bool mainloop( uint32_t frame );

		// Some helper methods
		virtual bool initialise( void );

		virtual bool exit( void );

	protected:
		// eq overriden, loop invariant
        virtual bool clientLoop (void);
		
		eq::ServerPtr _server;
		eqOgre::Config *_config;
		eqOgre::InitData _init_data;

    };	// class Client

}	// namespace eqOgre

#endif // EQ_OGRE_CLIENT_HPP
