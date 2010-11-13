/*	Joonatan Kuosa
 *	2010-05
 *
 *	Settings for the scene.
 *	Has settings for
 *	Ogre3D : resources and plugins
 *	equalizer : config file
 *	sceneloader : scenes to load
 *	tracking : config file
 */
#ifndef VL_SETTINGS_HPP
#define VL_SETTINGS_HPP

#include <string>

#include "base/args.hpp"
#include "base/rapidxml.hpp"
#include "base/filesystem.hpp"
#include "base/typedefs.hpp"

#include "base/projsettings.hpp"
#include "base/envsettings.hpp"

namespace vl
{

class Settings
{
	public :
		Settings( EnvSettingsRefPtr env, ProjSettingsRefPtr proj );

		virtual ~Settings( void );

		void setLogDir( std::string const &dir )
		{ _log_dir = dir; }
		
		std::string const &getLogDir( void )
		{ return _log_dir; }

		virtual vl::Args &getEqArgs( void )
		{ return _eq_args; }

		virtual vl::Args const &getEqArgs( void ) const
		{ return _eq_args; }

		virtual void setExePath( std::string const &path );

		virtual void clear( void );

		ProjSettingsRefPtr getProjectSettings( void )
		{ return _proj; }

		void setProjectSettings( ProjSettingsRefPtr proj )
		{ _proj = proj; }

		EnvSettingsRefPtr getEnvironmentSettings( void )
		{ return _env; }

		void setProjectSettings( EnvSettingsRefPtr env )
		{ _env = env; }

		std::string getProjectName( void ) const
		{
			if( _proj )
			{ return _proj->getCasePtr()->getName(); }
			else
			{ return std::string(); }
		}

	protected :

		void updateArgs( void );

		// Log directory
		std::string _log_dir;

		// All the paths
		std::string _exe_path;
		vl::Args _eq_args;

		// Environment specific settings
		EnvSettingsRefPtr _env;
		
		// Project specific settings
		ProjSettingsRefPtr _proj;

};	// class Settings

}	// namespace vl

#endif
