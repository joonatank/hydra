/**
 *	Copyright (c) 2010 - 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-12
 *	@file resource.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

/**	Class to hold memory block where data from resource is stored
 *	@date 2011-05 Added ResourceStream for reading and writing to Resources
 */

// Interface
#include "resource.hpp"

// Necessary for memcpy
#include <cstring>

// Necessary for replacing line endings
#include "base/string_utils.hpp"
// Necessary for excpetions
#include "base/exceptions.hpp"

/// ----------------------- ResourceStream -----------------------------------
vl::ResourceStream::ResourceStream(vl::Resource *resource)
	: _resource(resource)
	, _index(0)
{
	assert(_resource);
}

void 
vl::ResourceStream::skip(int bytes)
{
	if(_index + bytes < 0)
	{ _index = 0; }

	_index += bytes;
	assert(_index <= _resource->size());
}

bool 
vl::ResourceStream::eof(void) const
{
	assert(_resource);
	
	return( _index >= _resource->size()-1 );
}

void
vl::ResourceStream::seek(size_t index)
{
	assert(_resource);
	assert(index <= _resource->size());

	_index = index;
}

size_t
vl::ResourceStream::read(char *mem, size_t bytes)
{
	assert(_resource);
	if(_index+bytes < _resource->size())
	{
		::memcpy(mem, _resource->get()+_index, bytes);
		_index += bytes;
		return bytes;
	}
	else
	{ return 0; }
}

size_t
vl::ResourceStream::write(char const *mem, size_t bytes)
{
	assert(_resource);
	std::clog << "vl::ResourceStream::write" << std::endl;
	BOOST_THROW_EXCEPTION(vl::not_implemented());
	// @todo real implementation, needs to increase the Memory buffer
	
	return 0;
}

std::string
vl::ResourceStream::getLine(bool trimAfter)
{
	assert(_resource);

	std::string str;
	bool ended = false;
	while(!ended)
	{
		char ch;
		if( read(ch) == 0 )
		{ 
			std::clog << "Hit End of file." << std::endl; 
			break; 
		}
		if( ch == '\n' )
		{ ended = true; }
		else
		{ str.push_back(ch); }
	}

	return str;
}

std::string
vl::ResourceStream::getName(void) const
{
	assert(_resource);
	return _resource->getName();
}

size_t 
vl::ResourceStream::left(void) const
{
	assert(_resource);
	return _resource->size()-(_index+1);
}


bool
vl::ResourceStream::isWriteable(void) const
{
	std::clog << "vl::ResourceStream::isWriteable" << std::endl;
	BOOST_THROW_EXCEPTION(vl::not_implemented());
	/// @todo real implementation
	return true;
}

/// --------------------------- Resource -------------------------------------
void
vl::Resource::set( char const *mem, size_t size )
{
	assert( (size == 0 && mem == 0) || size > 0 );
	_memory.resize(size);

	if( size > 0 )
	{ ::memcpy( &(_memory[0]), mem, size ); }
}

void
vl::Resource::set( std::vector<char> const &mem )
{
	_memory = mem;
}

/// ------------------ TextResource -----------------------
void
vl::TextResource::set( char const *mem, size_t size )
{
	if( mem )
	{
		std::string str(mem, size-1);
		vl::replace_line_endings(str);
		vl::Resource::set( str.c_str(), str.size()+1 );
	}
}

void
vl::TextResource::set( std::vector<char> const &mem )
{
	std::string str( mem[0], mem.size()-1 );
	vl::replace_line_endings(str);
	vl::Resource::set( str.c_str(), str.size()+1 );
}

/// ----------------- Global --------------------------------------------------
void
vl::loadResource ( vl::Resource& res, const std::string& path )
{
	// Open in binary mode, so we don't mess up the file
	// open it from the end so that we get the size with tellg
	// TODO move the copy to use iterator insert
	std::ifstream ifs( path.c_str(), std::ios::in | std::ios::binary | std::ios::ate );
	size_t size = ifs.tellg();
	// TODO change to read the file straight to resource without copying
	char *mem = new char[size+1];
	ifs.seekg( 0, std::ios::beg );
	ifs.read( mem, size );
	ifs.close();

	res.set( mem, size+1 );
};