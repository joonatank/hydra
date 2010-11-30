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

/// Class to hold memory block where binary data from resource is stored
class MemoryBlock
{

};

class ResourceManager
{
public :
	ResourceManager( void );

	~ResourceManager( void );

	bool addResourcePath( std::string const &resource_dir, bool recursive = true );

	std::vector<std::string> const &getResourcePaths( void )
	{ return _paths; }

	bool loadResource( std::string const &name, std::string &data );

	bool findResource( std::string const &name, std::string &path );

private :
	std::vector<std::string> _paths;


};	// class ResourceManager

}	// namepsace vl

#endif // VL_RESOURCE_MANAGER_HPP