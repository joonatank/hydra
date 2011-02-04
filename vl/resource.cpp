/**	Joonatan Kuosa
 *	2010-12
 */

// Interface
#include "resource.hpp"

// Necessary for memcpy
#include <cstring>
// Necessary for EQASSERT
#include <co/base/debug.h>
// Necessary for replacing line endings
#include "base/string_utils.hpp"

/// ------------------ Resource -----------------------
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