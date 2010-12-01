/**	Joonatan Kuosa
 *	2010-11
 *
 */


// Declaration
#include "eq_resource.hpp"

// Needed for memcpy
#include <cstring>


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
eqOgre::Resource::copy( const vl::Resource& other )
{
	_name = other.getName();

	_block.destroy();
	if( other.getRawMemory().size > 0 )
	{
		_block.size = other.getRawMemory().size;
		_block.mem = new char[_block.size];
		::memcpy( _block.mem, other.getRawMemory().mem, _block.size );
	}
}

