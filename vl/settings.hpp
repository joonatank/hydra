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
 *
 *	2011-02
 *	Removed EnvSettings from here. Now this works only as a container for multiple
 *	project settings, where projects can be retrieved, added and destroyed.
 *	This provides auxialiary functions for retrieving the information from all
 *	project settings in one container.
 **/
#ifndef VL_SETTINGS_HPP
#define VL_SETTINGS_HPP

// Necessary because most parameters are strings
#include <string>

// Necessary for getEqArgs
// #include "base/args.hpp"

// Necessary for ProjSettings::Scene
#include "base/projsettings.hpp"

namespace vl
{

class Settings
{
	public :
		/**	Constructor
		 *	Pass the environment and project settings as parameters.
		 *
		 *	Parameters :
		 *		valid env and proj need to be present for master node settings
		 *		global is obtional for master node
		 *
		 *	Default constructor is provided for slave nodes
		 *	Same behaviour would result also with omiting either env or proj.
		 */
		Settings( ProjSettings const &proj, ProjSettings const &global );

		Settings( ProjSettings const &proj );

		Settings( void );

		virtual ~Settings( void );

		ProjSettings const &getProjectSettings( void ) const
		{ return _proj; }

		void setProjectSettings( ProjSettings const &proj )
		{ _proj = proj; }

		/// This supports multiple Project settings using the auxilary function
		/// multiple settings can be added and they are automatically used
		/// when retrieving resource paths, scripts or scenes.
		void addAuxilarySettings( ProjSettings const &proj );

		std::vector<ProjSettings> const &getAuxilarySettings( void ) const
		{ return _aux_projs; }

// 		ProjSettingsRefPtr getGlobalSettings( void )
// 		{ return _global; }
//
// 		void setGlobalSettings( ProjSettingsRefPtr proj )
// 		{ _global = proj; }


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
// 		std::string getEqLogFilePath( PATH_TYPE const type = PATH_ABS ) const;


		/**	Get the path to Ogre log file
		 *	Returns a filename which is in the log dir and has the project and pid
		 *	If no project name is set will substitute unamed for project name
		 *
		 *	Use type parameter to chose if the path returned is
		 *	relative to the exe or an absolute path
		 *	Defaults to returning an absolute path
		 */
// 		std::string getOgreLogFilePath( PATH_TYPE const type = PATH_ABS ) const;
//
// 		/// Get the path to log file relative to the exe
// 		/// Parameters: identifier can be used to distinquish libraries
// 		/// 			prefix can be used to add a prefix like debug to the file
// 		/// Returns a filename which is in the log dir and has the project and pid
// 		/// If no project name is set will substitute unamed for project name
// 		std::string getLogFilePath( std::string const &identifier,
// 									std::string const &prefix = std::string(),
// 									PATH_TYPE const type = PATH_ABS )
// 									const;

		/// Get the tracking files
		/// Returns a vector of the names of the tracking files
		/// Only tracking files that are in use are returned
// 		std::vector<std::string> getTrackingFiles( void ) const;

		/// Combines Global, the Project and the Case scenes to one vector
		/// Only scenes that are in use are added
		///
		/// Scene information structure is guarantied to contain
		/// - Scene name
		/// - scene file name
		/// - name of the scene this scene should be attached to
		/// - name of the SceneNode to whom this scene should be attached
		///
		/// Returns a vector of Scene info structures.
		std::vector<ProjSettings::Scene> getScenes( void ) const;

		/// Combines Global, the Project and the Case scripts to one vector
		/// Only scripts that are in use are added
		///
		/// Script information needed to pass from this class
		/// Script file, absolute path
		/// Returns a vector of script names
		std::vector<std::string> getScripts( void ) const;

		std::vector<std::string> getAuxDirectories( void ) const;

		std::string getProjectDir( void ) const;

		bool empty( void ) const;

	protected :
		void _addScripts( std::vector<std::string> &vec,
						  vl::ProjSettings::Case const &cas ) const;

		void _addScenes( std::vector<ProjSettings::Scene> &vec,
						vl::ProjSettings::Case const &cas ) const;

		// Project specific settings
		ProjSettings _proj;

		// Global project settings
		std::vector<ProjSettings> _aux_projs;

		// Name of the current case or empty if doesn't have a case
		std::string _case;

};	// class Settings

}	// namespace vl

#endif	// VL_SETTINGS_HPP
