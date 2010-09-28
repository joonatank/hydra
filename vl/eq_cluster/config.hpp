#ifndef EQ_OGRE_CONFIG_H
#define EQ_OGRE_CONFIG_H

#include <eq/eq.h>

#include "frame_data.hpp"
#include "init_data.hpp"
#include "settings.hpp"

namespace eqOgre
{
    class Config : public eq::Config
    {
    public:
		Config( eq::base::RefPtr< eq::Server > parent );

		/** @sa eq::Config::init. */
		virtual bool init( uint32_t const initID );

		/** @sa eq::Config::exit. */
//		virtual bool exit (void);

        /** @sa eq::Config::handleEvent */
//		virtual bool handleEvent( const eq::ConfigEvent* event );

		virtual uint32_t startFrame (const uint32_t frameID);

		void setInitData( InitData const &data )
		{ _init_data = data; }

		InitData const &getInitData( void )
		{ return _init_data; }

		void mapData( uint32_t const initDataID );

		void unmapData( void );
		
		void setSettings( vl::SettingsRefPtr set )
		{ _settings = set; }

		vl::SettingsRefPtr getSettings( void )
		{ return _init_data.getSettings(); }

	protected :
		virtual ~Config (void);

		vl::SettingsRefPtr _settings;

		InitData _init_data;
		FrameData _frame_data;
    };	// class Config

}	// namespace eqOgre

#endif // EQ_OGRE_CONFIG_H
