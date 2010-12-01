
#include "eq_resource.hpp"

#include <cstring>

#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>

eqOgre::Resource::Resource(const std::string& name)
	: _name(name)
{}


eqOgre::Resource::~Resource(void )
{
	_block.destroy();
}

void
eqOgre::Resource::setRawMemory(vl::MemoryBlock const &block)
{
	_block.destroy();
	_block = block;
}

void
eqOgre::Resource::setRawMemory(char* mem, size_t size)
{
	_block.destroy();
	_block = vl::MemoryBlock( mem, size );
}

vl::MemoryBlock
eqOgre::Resource::release( void )
{
	vl::MemoryBlock tmp( _block );
	_block = vl::MemoryBlock();

	return tmp;
}

void
eqOgre::Resource::copy( const eqOgre::Resource& other )
{
	_name = other._name;

	_block.destroy();
	if( other._block.size > 0 )
	{
		_block.size = other._block.size;
		_block.mem = new char[_block.size];
		::memcpy( _block.mem, other._block.mem, _block.size );
	}
}


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
	_waiting_for_loading.push_back(name);
}

void
eqOgre::ResourceManager::removeResource(const std::string& name)
{
	// Not a priority
	EQASSERTINFO( false, "NOT implemented yet." );
}

void
eqOgre::ResourceManager::loadAllResources(void )
{
	size_t offset = _resources.size();
	_resources.resize( offset + _waiting_for_loading.size() );

	for( size_t i = 0; i < _waiting_for_loading.size(); ++i )
	{
		// Basic error checking abort if fails
		EQASSERT( loadResource( _waiting_for_loading.at(i), _resources.at(offset+i) ) );
	}
}

/// ------ Protected ------
void
eqOgre::ResourceManager::getInstanceData(eq::net::DataOStream& os)
{
	// Serialize resource paths, used by Ogre
	os << _paths;

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
	is >> _paths;

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
