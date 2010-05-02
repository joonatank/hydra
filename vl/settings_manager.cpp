#include "settings_manager.hpp"

vl::SettingsManager::SettingsManager( void )
{
}

vl::SettingsManager::~SettingsManager( void )
{
}

void
vl::SettingsManager::setSettings( vl::Settings const &set )
{
	_settings = set;
}

