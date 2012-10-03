/**
 *	Copyright (c) 2010 - 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-11
 *	@file resource_manager.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

// Declaration
#include "resource_manager.hpp"

// Needed for boots::filesystem::path
#include "base/filesystem.hpp"
// Needed for resource file loading
#include <fstream>

// Necessary for exceptions
#include "base/exceptions.hpp"
#include "logger.hpp"


/// ------------ ResourceManager --------------
vl::ResourceManager::ResourceManager(void)
{}

vl::ResourceManager::~ResourceManager(void)
{}

void
vl::ResourceManager::loadResource( const std::string &name, vl::Resource &data )
{
	fs::path file(name);
	if( file.extension() == ".scene" )
	{
		vl::TextResource &scene_res = dynamic_cast<vl::TextResource &>( data );
		loadSceneResource( name, scene_res );
	}
	else if( file.extension() == ".ogg" )
	{
		loadOggResource( name, data );
	}
	else if( file.extension() == ".py" )
	{
		vl::TextResource &python_res = dynamic_cast<vl::TextResource &>( data );
		loadPythonResource( name, python_res );
	}
	else if( file.extension() == ".mesh" )
	{
		loadMeshResource(name, data);
	}
	else
	{
		std::cout << "Loading Generic Resource " << name << std::endl;
		// Generic resources are not added to any stack
		std::string file_path;
		if( !findResource(name, file_path) )
		{ BOOST_THROW_EXCEPTION( vl::missing_resource() << vl::resource_name(name) ); }

		_loadResource( name, file_path, data );
	}
}

void
vl::ResourceManager::loadSceneResource(const std::string& name, vl::TextResource& data)
{
	std::cout << vl::TRACE << "Loading Scene Resource " << name << std::endl;

	std::string extension(".scene");
	std::string scene_name = _stripExtension(name, extension);
	std::string file_name = _getFileName(name, extension);

	std::string file_path;
	if( !findResource( file_name, file_path ) )
	{ BOOST_THROW_EXCEPTION( vl::missing_resource() << vl::resource_name(file_name) ); }

	_loadResource( scene_name, file_path, data );
//	_scenes.push_back( vl::TextResource(data) );
}

void
vl::ResourceManager::loadPythonResource(const std::string& name, vl::TextResource& data)
{
	std::cout << vl::TRACE << "Loading Python Resource " << name << std::endl;

	std::string extension(".py");
	std::string script_name = _stripExtension(name, extension);
	std::string file_name = _getFileName(name, extension);

	std::string file_path;
	if( !findResource( file_name, file_path ) )
	{ BOOST_THROW_EXCEPTION( vl::missing_resource() << vl::resource_name(file_name) ); }

	_loadResource( script_name, file_path, data );
}

void
vl::ResourceManager::loadOggResource(const std::string& name, vl::Resource& data)
{
	std::cout << vl::TRACE << "Loading Ogg Resource " << name << std::endl;

	std::string extension(".ogg");
	std::string ogg_name = _stripExtension(name, extension);
	std::string file_name = _getFileName(name, extension);

	std::string file_path;
	if( !findResource( file_name, file_path ) )
	{ BOOST_THROW_EXCEPTION( vl::missing_resource() << vl::resource_name(file_name) ); }

	_loadResource( ogg_name, file_path, data );
}

void 
vl::ResourceManager::loadMeshResource(std::string const &name, vl::Resource &data)
{
	std::cout << vl::TRACE << "Loading Mesh Resource " << name << std::endl;

	std::string extension(".mesh");
	std::string mesh_name = _stripExtension(name, extension);
	std::string file_name = _getFileName(name, extension);
	
	std::string file_path;
	if( !findResource( file_name, file_path ) )
	{ BOOST_THROW_EXCEPTION( vl::missing_resource() << vl::resource_name(file_name) ); }

	_loadResource(mesh_name, file_path, data);
}

void
vl::ResourceManager::loadRecording(std::string const &name, vl::Resource &data)
{
	std::cout << vl::TRACE << "Loading Recording " << name << std::endl;

	// Doesn't have an extension so we assume the name olready has one
	
	std::string file_path;
	if( !findResource(name, file_path) )
	{ BOOST_THROW_EXCEPTION( vl::missing_resource() << vl::resource_name(name) ); }

	_loadResource(name, file_path, data);
}

bool
vl::ResourceManager::findResource(const std::string& name, std::string& path) const
{
	for( std::vector<std::string>::const_iterator iter = _search_paths.begin();
		 iter != _search_paths.end(); ++iter )
	{
		fs::path dir( *iter );
		fs::path file_path = dir/name;
		if( fs::exists( file_path ) )
		{

			path = file_path.string();
			return true;
		}
	}

	return false;
}

void
vl::ResourceManager::addResourcePath(std::string const &resource_dir, bool recursive)
{
	fs::path dir(resource_dir);
	if( !fs::exists(dir) || !fs::is_directory(dir) )
	{ BOOST_THROW_EXCEPTION( vl::missing_dir() << vl::file_name( resource_dir ) ); }

	// Never add path multiple times
	std::vector<std::string>::const_iterator iter
		= std::find( _search_paths.begin(), _search_paths.end(), resource_dir );
	if( iter == _search_paths.end() )
	{ _search_paths.push_back(resource_dir); }

	if( recursive )
	{
		fs::recursive_directory_iterator end_iter;
		for( fs::recursive_directory_iterator dir_iter(dir);
			 dir_iter != end_iter; ++dir_iter )
		{
			fs::path path( dir_iter->path() );
			if( fs::is_directory( path ) )
			{
				iter = std::find( _search_paths.begin(), _search_paths.end(), path.string() );
				if( iter == _search_paths.end() )
				{ _search_paths.push_back( dir_iter->path().string() ); }
			}
		}
	}
}

void
vl::ResourceManager::removeResourcePath(std::string const &resource_dir)
{
	std::clog << "vl::ResourceManager::removeResourcePath : NOT IMPLEMENTED" << std::endl;
}

std::vector< std::string > const &
vl::ResourceManager::getResourcePaths( void ) const
{
	return _search_paths;
}


/// ------------------------------ Protected ---------------------------------
void
vl::ResourceManager::_loadResource( std::string const &name,
										std::string const &path,
										vl::Resource &data ) const
{
	assert( fs::exists(path) );
	// Load the resource

	vl::loadResource( data, path );

	// Set the resource name
	data.setName( name );
}

std::string
vl::ResourceManager::_getFileName( std::string const &name,
									   std::string const &extension )
{
	if( fs::path(name).extension() == extension )
	{ return name; }
	else
	{ return name+extension; }
}

std::string
vl::ResourceManager::_stripExtension( std::string const &name,
										  std::string const &extension )
{
	if( fs::path(name).extension() == extension )
	{ return fs::path(name).stem().string(); }
	else
	{ return name; }
}
