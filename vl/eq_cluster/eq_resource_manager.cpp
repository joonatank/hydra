/**	Joonatan Kuosa
 *	2010-11
 *
 */

// Declaration
#include "eq_resource_manager.hpp"

// Needed for boots::filesystem::path
#include "base/filesystem.hpp"
// Needed for resource file loading
#include <fstream>

// Necessary for exceptions
#include "base/exceptions.hpp"

#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>

/// ------------ ResourceManager --------------
eqOgre::ResourceManager::ResourceManager( void )
{}

eqOgre::ResourceManager::~ResourceManager( void )
{}

std::vector< vl::TextResource > const &
eqOgre::ResourceManager::getSceneResources( void ) const
{
	return _scenes;
}


void
eqOgre::ResourceManager::addResource(const std::string& name)
{
	BOOST_THROW_EXCEPTION( vl::not_implemented() );
//	_waiting_for_loading.push_back(name);
}

void
eqOgre::ResourceManager::removeResource(const std::string& name)
{
	// Not a priority
	BOOST_THROW_EXCEPTION( vl::not_implemented() );
}

void
eqOgre::ResourceManager::loadAllResources( void )
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
eqOgre::ResourceManager::loadResource( const std::string &name, vl::Resource &data )
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
	else
	{
		EQINFO << "Loading Generic Resource " << name << std::endl;
		// Generic resources are not added to any stack
		std::string file_path;
		if( !findResource(name, file_path) )
		{ BOOST_THROW_EXCEPTION( vl::missing_resource() << vl::resource_name(name) ); }

		_loadResource( file_path, data );
		data.setName( name );
	}
}

void
eqOgre::ResourceManager::loadSceneResource(const std::string& name, vl::TextResource& data)
{
	EQINFO << "Loading Scene Resource " << name << std::endl;

	std::string file_path;

	fs::path file_name(name);

	// TODO should be tested
	if( fs::path(name).extension() != ".scene" )
	{
		file_name = name + ".scene";
	}

	if( !findResource(name, file_path) )
	{ BOOST_THROW_EXCEPTION( vl::missing_resource() << vl::resource_name(name) ); }

	_loadResource( file_path, data );
	data.setName( name );
	_scenes.push_back( vl::TextResource(data) );
}


bool
eqOgre::ResourceManager::findResource(const std::string& name, std::string& path) const
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
eqOgre::ResourceManager::addResourcePath( std::string const &resource_dir, bool recursive )
{
	fs::path dir(resource_dir);
	if( !fs::exists(dir) || !fs::is_directory(dir) )
	{ BOOST_THROW_EXCEPTION( vl::missing_dir() << vl::file_name( resource_dir ) ); }

	_search_paths.push_back(resource_dir);
	if( recursive )
	{
		fs::recursive_directory_iterator end_iter;
		for( fs::recursive_directory_iterator iter(dir);
			 iter != end_iter; ++iter )
		{
			if( fs::is_directory( iter->path() ) )
			{
				_search_paths.push_back( iter->path().file_string() );
			}
		}
	}
}


std::vector< std::string > const &
eqOgre::ResourceManager::getResourcePaths( void ) const
{
	return _search_paths;
}


/// ------ Protected ------
bool
eqOgre::ResourceManager::_findLoadedResource( const std::string& res_name,
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
eqOgre::ResourceManager::_loadResource( const std::string &path, vl::Resource &data ) const
{
	EQASSERT( fs::exists(path) );
	// Load the resource

	// Open in binary mode, so we don't mess up the file
	// open it from the end so that we get the size with tellg
	// TODO move the copy to use iterator insert
	std::ifstream ifs( path.c_str(), std::ios::in | std::ios::binary | std::ios::ate );
	size_t size = ifs.tellg();
	char *mem = new char[size+1];
	ifs.seekg( 0, std::ios::beg );
	ifs.read( mem, size );
	ifs.close();

	data.set( mem, size+1 );
}


void
eqOgre::ResourceManager::getInstanceData(eq::net::DataOStream& os)
{
	// Serialize resource paths, used by Ogre
	os << _search_paths;

	// Serialize all loaded resources
	os << _scenes.size();
	for( size_t i = 0; i < _scenes.size(); ++i )
	{
		operator<<( _scenes.at(i), os );
	}
}


void
eqOgre::ResourceManager::applyInstanceData(eq::net::DataIStream& is)
{
	// Deserialize resource paths, used by Ogre
	is >> _search_paths;

	// Deserialize all loaded resources
	size_t size;
	is >> size;
	_scenes.resize(size);
	for( size_t i = 0; i < size; ++i )
	{
		operator>>( _scenes.at(i), is );
	}
}



/// ---------- Global -----------
eq::net::DataOStream &
eqOgre::operator<<(vl::Resource& res, eq::net::DataOStream& os)
{
	os << res.getName();

	os << res.size();
	for( size_t i = 0; i < res.size(); ++i )
	{
		os << res.get()[i];
	}

	return os;
}

eq::net::DataIStream &
eqOgre::operator>>(vl::Resource& res, eq::net::DataIStream& is)
{
	std::string name;
	size_t size;
	is >> name >> size;

	res.setName(name);

	res.resize(size);
	for( size_t i = 0; i < size; ++i )
	{
		is >> res[i];
	}

	return is;
}
