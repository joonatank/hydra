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


std::string
createLogFilePath( const std::string &project_name,
					   const std::string &identifier,
					   const std::string &prefix = std::string(),
					   const std::string &log_dir = std::string() );

class Settings
{
	public :
		enum PATH_TYPE
		{
			PATH_ABS,
			PATH_REL
		};

		Settings( EnvSettingsRefPtr env, ProjSettingsRefPtr proj,
				  ProjSettingsRefPtr global = ProjSettingsRefPtr() );

		virtual ~Settings( void );

		/// Set wether or not suppress output to std::cerr
		/// If set to true the application will print to std::cerr
		/// instead of or in addition to printing to log file
		void setVerbose( bool verbose )
		{ _verbose = verbose; }

		/// Wether we are outputing to std::cerr
		/// If true will print to std::cerr, if false will not
		///
		/// Does not define anything about log files, the application might
		/// print to log files even when set true or it might not
		bool getVerbose( void ) const
		{ return _verbose; }

		/// Set the directory logs are stored
		/// Path is assumed to be relative, though absolute might work it's
		/// not guaranteed.
		void setLogDir( std::string const &dir )
		{ _log_dir = dir; }

		/// Get the directory logs are stored.
		/// Relative and absolute can be chosen using type parameter
		/// Defaults to returning absolute path
		std::string getLogDir( PATH_TYPE const type = PATH_ABS ) const;

		vl::Args &getEqArgs( void )
		{ return _eq_args; }

		vl::Args const &getEqArgs( void ) const
		{ return _eq_args; }

		/// Set the exe path i.e. the command used to start the program
		void setExePath( std::string const &path );

		ProjSettingsRefPtr getGlobalSettings( void )
		{ return _global; }

		void setGlobalSettings( ProjSettingsRefPtr proj )
		{ _global = proj; }


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

		/**	Get the path to Equalizer log file
		 *	Returns a filename which is in the log dir and has the project and pid
		 *	If no project name is set will substitute unamed for project name
		 *
		 *	Use type parameter to chose if the path returned is
		 *	relative to the exe or an absolute path
		 *	Defaults to returning an absolute path
		 */
		std::string getEqLogFilePath( PATH_TYPE const type = PATH_ABS ) const;


		/**	Get the path to Ogre log file
		 *	Returns a filename which is in the log dir and has the project and pid
		 *	If no project name is set will substitute unamed for project name
		 *
		 *	Use type parameter to chose if the path returned is
		 *	relative to the exe or an absolute path
		 *	Defaults to returning an absolute path
		 */
		std::string getOgreLogFilePath( PATH_TYPE const type = PATH_ABS ) const;

		/// Get the path to log file relative to the exe
		/// Parameters: identifier can be used to distinquish libraries
		/// 			prefix can be used to add a prefix like debug to the file
		/// Returns a filename which is in the log dir and has the project and pid
		/// If no project name is set will substitute unamed for project name
		std::string getLogFilePath( std::string const &identifier,
									std::string const &prefix = std::string(),
									PATH_TYPE const type = PATH_ABS )
									const;

		std::vector<std::string> getTrackingPaths( void ) const;


		/// Combines Global, the Project and the Case scenes to one vector
		/// Only scenes that are in use are added
		///
		/// Scene information needed to pass from this class
		/// Name, File (to load), attach scene and attach point
		///
		/// Returns a vector of the used scene pointers
		/// Scenes returned are owned by ProjectSettings, ownership is not passed
		std::vector<ProjSettings::Scene const *> getScenes( void ) const;

		/// Combines Global, the Project and the Case scripts to one vector
		/// Only scripts that are in use are added
		///
		/// Script information needed to pass from this class
		/// Script file, absolute path
		/// Returns a vector of script file paths, absolute
		std::vector<std::string> getScripts( void ) const;

		std::vector<std::string> getResourcePaths( void ) const;

		std::string getGlobalDir( void ) const;

		std::string getProjectDir( void ) const;

		std::string getEnvironementDir( void ) const;

	protected :
		void _addScripts( std::vector<std::string> &vec, std::string const &projDir,
						  vl::ProjSettings::Case const *cas ) const;

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

		// Global project settings
		ProjSettingsRefPtr _global;

		// Project specific settings
		ProjSettingsRefPtr _proj;

		// Name of the current case or empty if doesn't have a case
		std::string _case;

		bool _verbose;

};	// class Settings

}	// namespace vl

#endif
