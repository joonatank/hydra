
#include "eq_resource.hpp"

#include <cstring>

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
	_block.destroy();
	if( other._block.size > 0 )
	{
		_block.size = other._block.size;
		_block.mem = new char[_block.size];
		::memcpy( _block.mem, other._block.mem, _block.size );
	}
}
