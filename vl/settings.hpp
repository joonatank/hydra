/*	Joonatan Kuosa
 *	2010-05
 *
 *	Settings for the scene.
 *	Has settings for both ogre, sceneloader and equalizer.
 */
#ifndef VL_SETTINGS_HPP
#define VL_SETTINGS_HPP

#include <string>

namespace vl
{

class Settings
{
	public :
		Settings( void );

		~Settings( void );

	private :
		std::string _file_path;
		std::string _eq_config_path;
		std::string _scene_file_path;

};	// class Settings

class SettingsSerializer
{
	public :
		SettingsSerializer( void );

		~SettingsSerializer( void );

	private :
};

class SettingsDeserializer
{
	public :
		SettingsDeserializer( void );

		~SettingsDeserializer( void );

	private :

};

}	// namespace vl

#endif
