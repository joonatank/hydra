/**
 *	Copyright (c) 2010 - 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2013 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-05
 *	@file settings.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

// Declaration
#include "settings.hpp"

// Necessary for exceptions
#include "base/exceptions.hpp"

#include "base/envsettings.hpp"


vl::Settings::Settings(vl::config::ProjSettings const &proj, vl::config::ProjSettings const &global)
	: _proj(proj)
{
	addAuxilarySettings(global);
}

vl::Settings::Settings(vl::config::ProjSettings const &proj)
	: _proj(proj)
{}

vl::Settings::Settings( void )
{}


vl::Settings::~Settings( void )
{}

void
vl::Settings::addAuxilarySettings(vl::config::ProjSettings const &proj)
{
	// TODO should check that the same is not added twice
	_aux_projs.push_back(proj);
}

std::string
vl::Settings::getProjectName( void ) const
{
	return _proj.getCase().getName();
}

std::vector<vl::config::ProjSettings::Scene>
vl::Settings::getScenes( void ) const
{
	std::vector<vl::config::ProjSettings::Scene> scenes;

	for( size_t i = 0; i < _aux_projs.size(); ++i )
	{ _addScenes( scenes, _aux_projs.at(i).getCase() ); }

	_addScenes( scenes, _proj.getCase() );

	if( !_case.empty() )
	{ _addScenes( scenes, _proj.getCase(_case) ); }

	return scenes;
}

std::vector< std::string >
vl::Settings::getScripts( void ) const
{
	std::vector<std::string> vec;

	for( size_t i = 0; i < _aux_projs.size(); ++i )
	{ _addScripts( vec, _aux_projs.at(i).getCase() ); }

	_addScripts( vec, _proj.getCase() );

	if( !_case.empty() )
	{ _addScripts( vec, _proj.getCase(_case) ); }

	return vec;
}

std::vector<std::string>
vl::Settings::getAuxDirectories(void ) const
{
	std::vector<std::string> paths;
	for( size_t i = 0; i < _aux_projs.size(); ++i )
	{
		// Never should there be extra projects without filenames
		// This will be relaxed when the projects can be created using the editor
		if(_aux_projs.at(i).getFile().empty())
		{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Empty project file")); }

		fs::path file( _aux_projs.at(i).getFile() );
		fs::path dir = file.parent_path();
		if( !fs::exists( dir ) )
		{ BOOST_THROW_EXCEPTION( vl::missing_dir() << vl::file_name( dir.string() ) ); }
		paths.push_back(dir.string());
	}

	return paths;
}

std::string
vl::Settings::getProjectDir( void ) const
{
	fs::path projFile( _proj.getFile() );
	fs::path projDir = projFile.parent_path();

	return projDir.string();
}

bool
vl::Settings::empty( void ) const
{
	return( _proj.empty() && _aux_projs.empty() );
}

bool
vl::Settings::hasProject(const std::string& name) const
{
	if( _proj.getName() == name )
	{ return true; }

	std::vector<vl::config::ProjSettings>::const_iterator iter;
	for( iter = _aux_projs.begin(); iter != _aux_projs.end(); ++iter )
	{
		if( iter->getName() == name )
		{ return true; }
	}

	return false;
}

vl::config::ProjSettings const &
vl::Settings::findProject( std::string const &name ) const
{
	if( _proj.getName() == name )
	{ return _proj; }

	std::vector<vl::config::ProjSettings>::const_iterator iter;
	for( iter = _aux_projs.begin(); iter != _aux_projs.end(); ++iter )
	{
		if( iter->getName() == name )
		{ return *iter; }
	}

	// TODO replace with correct exception
	std::string desc( "No such project found" );
	BOOST_THROW_EXCEPTION( vl::exception() << vl::desc(desc) );
}

std::string
vl::Settings::getDir(vl::config::ProjSettings const &proj) const
{
	fs::path projFile( proj.getFile() );
	fs::path projDir = projFile.parent_path();

	return projDir.string();
}


/// -------------------- Settings Protected ------------------------------------
void
vl::Settings::_addScripts( std::vector< std::string > &vec,
						   vl::config::ProjSettings::Case const &cas ) const
{
	for( size_t i = 0; i < cas.getNscripts(); ++i )
	{
		vl::config::ProjSettings::Script const &script = cas.getScript(i);
		if( script.getUse() )
		{
			vec.push_back( script.getFile() );
		}
	}
}


void
vl::Settings::_addScenes(std::vector<vl::config::ProjSettings::Scene> &vec,
						 vl::config::ProjSettings::Case const &cas ) const
{
	for( size_t i = 0; i < cas.getNscenes(); ++i )
	{
		vl::config::ProjSettings::Scene const &scene = cas.getScene(i);
		if( scene.getUse() )
		{
			vec.push_back( scene );
		}
	}
}
