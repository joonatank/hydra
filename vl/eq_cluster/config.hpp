#ifndef EQ_OGRE_CONFIG_H
#define EQ_OGRE_CONFIG_H

#include <eq/eq.h>

#include "settings.hpp"

namespace eqOgre
{
    class Config : public eq::Config
    {
    public:
		Config( eq::base::RefPtr< eq::Server > parent );

		/** @sa eq::Config::init. */
//		virtual bool init( const uint32_t initID );

		/** @sa eq::Config::exit. */
//		virtual bool exit (void);

        /** @sa eq::Config::handleEvent */
//		virtual bool handleEvent( const eq::ConfigEvent* event );

		void setSettings( vl::SettingsRefPtr set )
		{ _settings = set; }

		vl::SettingsRefPtr getSettings( void )
		{ return _settings; }

	protected :
		virtual ~Config (void);

		vl::SettingsRefPtr _settings;

    };	// class Config

}	// namespace eqOgre

#endif // EQ_OGRE_CONFIG_H
