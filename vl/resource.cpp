/**	Joonatan Kuosa
 *	2010-12
 */
// Interface
#include "resource.hpp"

// Necessary for memcpy
#include <cstring>
// Necessary for EQASSERT
#include <co/base/debug.h>

/// ------------------ Resource -----------------------
void vl::Resource::set( char const *mem, size_t size )
{
	EQASSERT( (size == 0 && mem == 0) || size > 0 );
	_memory.resize(size);

	if( size > 0 )
	{ ::memcpy( &(_memory[0]), mem, size ); }
}

void vl::Resource::set( std::vector<char> const &mem )
{
	_memory = mem;
}

/// ------------------ TextResource -----------------------
void vl::TextResource::set( char const *mem, size_t size )
{
	vl::Resource::set(mem, size);

	// Convert the ending to NULL terminator
	if( size > 0 )
	{ _memory[size-1] = '\0'; }
}

void vl::TextResource::set( std::vector<char> const &mem )
{
	vl::Resource::set(mem);

	// Convert the ending to NULL terminator
	if( _memory.size() > 0 )
	{ _memory[_memory.size()-1] = '\0'; }
}
