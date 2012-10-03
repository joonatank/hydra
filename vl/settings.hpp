/**
 *	Copyright (c) 2010 - 2011 Tampere University of Technology
  *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-05
 *	@file settings.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

/**	
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

#ifndef HYDRA_SETTINGS_HPP
#define HYDRA_SETTINGS_HPP

// Necessary because most parameters are strings
#include <string>

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

		/// Returns the name of the project
		std::string getProjectName( void ) const;

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

		/// @brief get the main project directory
		/// @return path name in native format
		std::string getProjectDir( void ) const;

		bool empty( void ) const;

		bool hasProject( std::string const &name ) const;

		vl::ProjSettings const &findProject( std::string const &name ) const;

		std::string getDir( vl::ProjSettings const &proj ) const;

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

#endif	// HYDRA_SETTINGS_HPP
