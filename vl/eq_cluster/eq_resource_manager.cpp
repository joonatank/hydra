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

#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>

/// ------------ ResourceManager --------------
eqOgre::ResourceManager::ResourceManager( void )
{}

eqOgre::ResourceManager::~ResourceManager( void )
{}

eqOgre::Resource
eqOgre::ResourceManager::copyResource(const std::string& name) const
{
	for( std::vector<Resource>::const_iterator iter = _resources.begin();
		 iter != _resources.end(); ++iter )
	{
		if( iter->getName() == name )
		{ return *iter; }
	}

	EQASSERTINFO( false, "Resource not found" );
}


std::vector< eqOgre::Resource >
eqOgre::ResourceManager::getSceneResources( void ) const
{
	std::vector<eqOgre::Resource> vec;
	for( size_t i = 0; i < _resources.size(); ++i )
	{
		if( std::string::npos != _resources.at(i).getName().rfind(".scene") )
		{
			vec.push_back( _resources.at(i) );
		}
	}
	return vec;
}


void
eqOgre::ResourceManager::addResource(const std::string& name)
{
	EQASSERTINFO( false, "NOT implemented yet." );
//	_waiting_for_loading.push_back(name);
}

void
eqOgre::ResourceManager::removeResource(const std::string& name)
{
	// Not a priority
	EQASSERTINFO( false, "NOT implemented yet." );
}

bool
eqOgre::ResourceManager::loadAllResources( void )
{
	EQASSERTINFO( false, "NOT implemented yet." );
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
	return false;
}


bool
eqOgre::ResourceManager::loadResource( const std::string &name, vl::Resource &data )
{
	// Find the resource from already loaded stack
	if( _findResource( name, data ) )
	{ return true; }

	std::string file_path;

	if( findResource(name, file_path) )
	{
		// Load the resource

		// Open in binary mode, so we don't mess up the file
		// open it from the end so that we get the size with tellg
		std::ifstream ifs( file_path.c_str(), std::ios::in | std::ios::binary | std::ios::ate );
		size_t size = ifs.tellg();
		char *mem = new char[size+1];
		ifs.seekg( 0, std::ios::beg );
		ifs.read( mem, size );
		ifs.close();

		data.setRawMemory( mem, size+1 );
		data.setName( name );

		// Data loaded add to distribution stack
		// TODO this should check the type, so that we only add necessary ones
		// to the distribution stack.
		// TODO this can add the file multiple times to the stack
		_resources.push_back( eqOgre::Resource(data) );

		return true;
	}

	return false;
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

bool
eqOgre::ResourceManager::addResourcePath( std::string const &resource_dir, bool recursive )
{
	fs::path dir(resource_dir);
	if( !fs::exists(dir) || !fs::is_directory(dir) )
	{ return false; }

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

	return true;
}


std::vector< std::string > const &
eqOgre::ResourceManager::getResourcePaths( void ) const
{
	return _search_paths;
}


/// ------ Protected ------
bool
eqOgre::ResourceManager::_findResource( const std::string& res_name, vl::Resource& resource ) const
{
	for( std::vector<Resource>::const_iterator iter = _resources.begin();
		 iter != _resources.end(); ++iter )
	{
		if( iter->getName() == res_name )
		{
			resource.copy(*iter);
			return true;
		}
	}

	return false;
}


void
eqOgre::ResourceManager::getInstanceData(eq::net::DataOStream& os)
{
	// Serialize resource paths, used by Ogre
	os << _search_paths;

	// Serialize all loaded resources
	os << _resources.size();
	for( size_t i = 0; i < _resources.size(); ++i )
	{
		operator<<( _resources.at(i), os );
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
	_resources.resize(size);
	for( size_t i = 0; i < size; ++i )
	{
		operator>>( _resources.at(i), is );
	}
}



/// ---------- Global -----------
eq::net::DataOStream &
eqOgre::operator<<(eqOgre::Resource& res, eq::net::DataOStream& os)
{
	os << res.getName();
	os << res.getRawMemory().size;
	for( size_t i = 0; i < res.getRawMemory().size; ++i )
	{
		os << res.getRawMemory().mem[i];
	}

	return os;
}

eq::net::DataIStream &
eqOgre::operator>>(eqOgre::Resource& res, eq::net::DataIStream& is)
{
	std::string name;
	size_t size;
	is >> name >> size;

	char *mem = new char[size];
	for( size_t i = 0; i < size; ++i )
	{
		is >> mem[i];
	}
	res.setName(name);
	res.setRawMemory(mem, size);

	return is;
}
