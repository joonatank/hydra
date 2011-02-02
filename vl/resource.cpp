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
