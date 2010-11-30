/**	Joonatan Kuosa
 *	2010-11
 *
 *	Class to load resources
 *	Holds a number of resource paths from where the resources are searched
 *	Loads ascii file resources to std::strings
 *
 *	Loads binary resources to memory
 *	TODO this is incomplete and not in use yet
 */

#ifndef VL_RESOURCE_MANAGER_HPP
#define VL_RESOURCE_MANAGER_HPP

#include <vector>
#include <string>

namespace vl
{

/// Container to pass around pointer and size pair
/// Does not do any memory handling
struct MemoryBlock
{
	MemoryBlock( char *m = 0, size_t siz = 0 )
		: mem(m), size(siz)
	{}

	/// Purposefully empty destructor, call destroy if you want to realease memory
	~MemoryBlock( void )
	{}

	/// Puporsefully copies a pointer
	MemoryBlock( MemoryBlock const &other )
		: mem(other.mem), size(other.size)
	{}

	MemoryBlock &operator=( MemoryBlock const &other )
	{
		mem = other.mem;
		size = other.size;
		return *this;
	}

	void setRawMemory( char *m, size_t siz )
	{ mem = m; size = siz; }

	void destroy( void )
	{ delete [] mem; size = 0; mem = 0; }

	char *mem;
	size_t size;
};

/**	Class to hold memory block where data from resource is stored
 *	Data is held in a raw binary form and is accessable in that form
 *	This is to not assume anything about any data, for example text files might
 *	not be in ascii format and assuming so might break them.
 *
 *	This class owns the memory passed to it
 *	if the release is called the ownership is passed to caller
 */
class Resource
{
public :
	/// Pass the ownership of a raw memory block to this resource
	virtual void setRawMemory( MemoryBlock const &block ) = 0;
	virtual void setRawMemory( char *mem, size_t size ) = 0;

	/// Get the ownership of the memory hold by the Resource
	/// After this the Resource has no memory block stored anymore
	/// Return value : valid memory block
	virtual MemoryBlock release( void ) = 0;

};

class ResourceManager
{
public :
	ResourceManager( void );

	virtual ~ResourceManager( void );

	bool addResourcePath( std::string const &resource_dir, bool recursive = true );

	std::vector<std::string> const &getResourcePaths( void )
	{ return _paths; }

	bool loadResource( std::string const &name, std::string &data );

	bool loadResource( std::string const &name, vl::Resource &data );

	bool findResource( std::string const &name, std::string &path );

private :
	std::vector<std::string> _paths;


};	// class ResourceManager

}	// namepsace vl

#endif // VL_RESOURCE_MANAGER_HPP