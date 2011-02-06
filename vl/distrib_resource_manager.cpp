/**	Joonatan Kuosa
 *	2010-11
 *
 */

// Declaration
#include "distrib_resource_manager.hpp"

// Needed for boots::filesystem::path
#include "base/filesystem.hpp"
// Needed for resource file loading
#include <fstream>

// Necessary for exceptions
#include "base/exceptions.hpp"


/// ------------ ResourceManager --------------
vl::DistribResourceManager::DistribResourceManager( void )
{}

vl::DistribResourceManager::~DistribResourceManager( void )
{}

std::vector< vl::TextResource > const &
vl::DistribResourceManager::getSceneResources( void ) const
{
	return _scenes;
}


void
vl::DistribResourceManager::addResource(const std::string& name)
{
	BOOST_THROW_EXCEPTION( vl::not_implemented() );
//	_waiting_for_loading.push_back(name);
}

void
vl::DistribResourceManager::removeResource(const std::string& name)
{
	// Not a priority
	BOOST_THROW_EXCEPTION( vl::not_implemented() );
}

void
vl::DistribResourceManager::loadAllResources( void )
{
	BOOST_THROW_EXCEPTION( vl::not_implemented() );
	/*
	bool retval = true;
	size_t offset = _resources.size();
	_resources.resize( offset + _waiting_for_loading.size() );

	for( size_t i = 0; i < _waiting_for_loading.size(); ++i )
	{
		// Basic error checking abort if fails
		retval = retval && loadResource( _waiting_for_loading.at(i), _resources.at(offset+i) );
	}

	return retval;
	*/
}


void
vl::DistribResourceManager::loadResource( const std::string &name, vl::Resource &data )
{
	// Find the resource from already loaded stack
	if( _findLoadedResource( name, data ) )
	{ return; }

	// TODO this should call the respective load functions for every file type
	fs::path file( name );
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
vl::DistribResourceManager::loadSceneResource(const std::string& name, vl::TextResource& data)
{
	std::cout << "Loading Scene Resource " << name << std::endl;

	std::string extension(".scene");
	std::string scene_name = _stripExtension(name, extension);
	std::string file_name = _getFileName(name, extension);

	std::string file_path;
	if( !findResource( file_name, file_path ) )
	{ BOOST_THROW_EXCEPTION( vl::missing_resource() << vl::resource_name(file_name) ); }

	_loadResource( scene_name, file_path, data );
	_scenes.push_back( vl::TextResource(data) );
}

void
vl::DistribResourceManager::loadPythonResource(const std::string& name, vl::TextResource& data)
{
	std::cout << "Loading Python Resource " << name << std::endl;

	std::string extension(".py");
	std::string script_name = _stripExtension(name, extension);
	std::string file_name = _getFileName(name, extension);

	std::string file_path;
	if( !findResource( file_name, file_path ) )
	{ BOOST_THROW_EXCEPTION( vl::missing_resource() << vl::resource_name(file_name) ); }

	_loadResource( script_name, file_path, data );
	_python_scripts.push_back( vl::TextResource(data) );
}

void
vl::DistribResourceManager::loadOggResource(const std::string& name, vl::Resource& data)
{
	std::cout << "Loading Ogg Resource " << name << std::endl;

	std::string extension(".ogg");
	std::string ogg_name = _stripExtension(name, extension);
	std::string file_name = _getFileName(name, extension);

	std::string file_path;
	if( !findResource( file_name, file_path ) )
	{ BOOST_THROW_EXCEPTION( vl::missing_resource() << vl::resource_name(file_name) ); }

	_loadResource( ogg_name, file_path, data );
	_ogg_sounds.push_back( vl::Resource(data) );
}



bool
vl::DistribResourceManager::findResource(const std::string& name, std::string& path) const
{
	for( std::vector<std::string>::const_iterator iter = _search_paths.begin();
		 iter != _search_paths.end(); ++iter )
	{
		fs::path dir( *iter );
		fs::path file_path = dir/name;
		if( fs::exists( file_path ) )
		{

			path = file_path.file_string();
			return true;
		}
	}

	return false;
}

void
vl::DistribResourceManager::addResourcePath( std::string const &resource_dir, bool recursive )
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
				iter = std::find( _search_paths.begin(), _search_paths.end(), path.file_string() );
				if( iter == _search_paths.end() )
				{ _search_paths.push_back( dir_iter->path().file_string() ); }
			}
		}
	}
}


std::vector< std::string > const &
vl::DistribResourceManager::getResourcePaths( void ) const
{
	return _search_paths;
}


/// ------ Protected ------
bool
vl::DistribResourceManager::_findLoadedResource( const std::string& res_name,
											  vl::Resource &resource ) const
{
	// TODO this should support other resource containers based on extension
	// e.g. python scripts, ogg files, scenes
	if( fs::path(res_name).extension() == ".scene" )
	{
		for( std::vector<vl::TextResource>::const_iterator iter = _scenes.begin();
			iter != _scenes.end(); ++iter )
		{
			if( iter->getName() == res_name )
			{
				resource = *iter;
				return true;
			}
		}
	}

	return false;
}

void
vl::DistribResourceManager::_loadResource( std::string const &name,
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
vl::DistribResourceManager::_getFileName( std::string const &name,
									   std::string const &extension )
{
	if( fs::path(name).extension() == extension )
	{ return name; }
	else
	{ return name+extension; }
}

std::string
vl::DistribResourceManager::_stripExtension( std::string const &name,
										  std::string const &extension )
{
	if( fs::path(name).extension() == extension )
	{ return fs::path(name).stem(); }
	else
	{ return name; }
}


/// --------------------------- Protected --------------------------------------
void
vl::DistribResourceManager::serialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits )
{
	// Serialize resource paths, used by Ogre
	if( dirtyBits & DIRTY_PATHS )
	{ msg << _search_paths; }

	// Serialize all loaded resources
	if( dirtyBits & DIRTY_SCENES )
	{ msg << _scenes; }
}


void
vl::DistribResourceManager::deserialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits )
{
	// Deserialize resource paths, used by Ogre
	if( dirtyBits & DIRTY_PATHS )
	{ msg >> _search_paths; }

	// Deserialize all loaded resources
	if( dirtyBits & DIRTY_SCENES )
	{ msg >> _scenes; }
}
