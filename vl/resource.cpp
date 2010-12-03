
#include "resource.hpp"

#include <cstring>

void vl::Resource::set(char* mem, size_t size)
{
	_memory.resize(size);
	::memcpy( &(_memory[0]), mem, size );
}

void vl::Resource::set(std::vector< char > mem)
{
	_memory = mem;
}


void vl::TextResource::set(char* mem, size_t size)
{
	vl::Resource::set(mem, size);

	// Convert the ending to NULL terminator
	_memory[size-1] = '\0';
}

void vl::TextResource::set(std::vector< char > mem)
{
	vl::Resource::set(mem);

	// Convert the ending to NULL terminator
	_memory[_memory.size()-1] = '\0';
}
