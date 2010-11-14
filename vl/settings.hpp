/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-05
 *
 *	Major Update 2010-11
 *	Master Settings file, has references to Project and Environment Settings
 *	Also contains file paths and necessary functions for starting the
 *	program.
 *
 *	Difference between this and other settings classes
 *	What is stored in here is never saved anywhere it's only for the current
 *	process.
 **/
#ifndef VL_SETTINGS_HPP
#define VL_SETTINGS_HPP

// Necessary because most parameters are strings
#include <string>

// Necessary for getEqArgs
#include "base/args.hpp"

#include "base/projsettings.hpp"
#include "base/envsettings.hpp"

namespace vl
{

class Settings
{
	public :
		Settings( EnvSettingsRefPtr env, ProjSettingsRefPtr proj );

		virtual ~Settings( void );

		/// Set the directory logs are stored
		/// Path is assumed to be relative, though absolute might work it's
		/// not guaranteed.
		void setLogDir( std::string const &dir )
		{ _log_dir = dir; }

		/// Get the directory logs are stored.
		/// Path is relative to the current directory
		// TODO all relative paths are bit problematic... they should return
		// absolute paths based on the paths that are relative to the exe
		std::string const &getLogDir( void ) const
		{ return _log_dir; }

		vl::Args &getEqArgs( void )
		{ return _eq_args; }

		vl::Args const &getEqArgs( void ) const
		{ return _eq_args; }

		/// Set the exe path i.e. the command used to start the program
		void setExePath( std::string const &path );

		ProjSettingsRefPtr getProjectSettings( void )
		{ return _proj; }

		void setProjectSettings( ProjSettingsRefPtr proj )
		{ _proj = proj; }

		EnvSettingsRefPtr getEnvironmentSettings( void )
		{ return _env; }

		void setProjectSettings( EnvSettingsRefPtr env )
		{ _env = env; }

		/// Returns the name of the project
		std::string getProjectName( void ) const;

		/// Get the path to Equalizer log file relative to the exe
		/// Returns a filename which is in the log dir and has the project and pid
		/// If no project name is set will substitute unamed for project name
		std::string getEqLogFilePath( void ) const;

		/// Get the path to Ogre log file relative to the exe
		/// Returns a filename which is in the log dir and has the project and pid
		/// If no project name is set will substitute unamed for project name
		std::string getOgreLogFilePath( void ) const;

		/// Get the path to log file relative to the exe
		/// Parameters: identifier can be used to distinquish libraries
		/// 			prefix can be used to add a prefix like debug to the file
		/// Returns a filename which is in the log dir and has the project and pid
		/// If no project name is set will substitute unamed for project name
		std::string getLogFilePath( std::string const &identifier,
									std::string const &prefix = std::string() )
									const;

		/// Combines the Project and the Case scenes to one vector
		/// Only scenes that are in use are added
		///
		/// Scene information needed to pass from this class
		/// Name, File (to load), attach scene and attach point
		///
		/// Returns a vector of the used scene pointers
		/// Scenes returned are owned by ProjectSettings, ownership is not passed
		std::vector<ProjSettings::Scene const *> getScenes( void ) const;

		/// Combines the Project and the Case script to one vector
		/// Only scripts that are in use are added
		///
		/// Script information needed to pass from this class
		/// Script file, absolute path
		/// Returns a vector of script file paths, absolute
		std::vector<std::string> getScripts( void ) const;

		std::string getProjectDir( void ) const;

		std::string getEnvironementDir( void ) const;

	protected :
		void _addScripts( std::vector<std::string> &vec, vl::ProjSettings::Case const *cas ) const;

		void _addScenes( std::vector<ProjSettings::Scene const *> &vec,
						vl::ProjSettings::Case const *cas ) const;
		
		void _updateArgs( void );

		// Log directory
		std::string _log_dir;

		// All the paths
		std::string _exe_path;
		vl::Args _eq_args;

		// Environment specific settings
		EnvSettingsRefPtr _env;
		
		// Project specific settings
		ProjSettingsRefPtr _proj;

		// Name of the current case or empty if doesn't have a case
		std::string _case;

};	// class Settings

}	// namespace vl

#endif
