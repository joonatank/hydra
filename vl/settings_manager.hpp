/*	Joonatan Kuosa
 *	2010-05
 *
 *	Class to manage the settings of the running program
 *	Manages multiple settings, changes between these settings and initing
 *	the system based on the settings.
 *
 *	Manages both equalizer configuration, Ogre configuration and our system
 *	configuration.
 */
#ifndef VL_SETTINGS_MANAGER_HPP
#define VL_SETTINGS_MANAGER_HPP

#include "settings.hpp"
#include <string>

namespace vl
{

class SettingsManager
{
	public :
		SettingsManager( void );

		~SettingsManager( void );

		void setSettings( vl::Settings const &set );

		vl::Settings &getSettings( void )
		{ return _settings; }

		vl::Settings const &getSettings( void ) const
		{ return _settings; }

	private :
		std::string _data_path;
		vl::Settings _settings;

};	// class SettingsManager

}	// namespace vl

#endif
