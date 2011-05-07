/**	Joonatan Kuosa
 *	2010-11
 *
 *	Class to load resources
 *	Holds a number of resource paths from where the resources are searched
 *	Loads ascii file resources to std::strings
 *
 *	Loads binary resources to memory
 */

#ifndef VL_RESOURCE_MANAGER_HPP
#define VL_RESOURCE_MANAGER_HPP

#include <vector>
#include <string>

#include "resource.hpp"

namespace vl
{

class ResourceManager
{
public :

	virtual void addResource( std::string const &name ) = 0;

	virtual void removeResource( std::string const &name ) = 0;

	virtual bool findResource( std::string const &name, std::string &path ) const = 0;

	virtual void loadResource( std::string const &name, vl::Resource &data ) = 0;

	virtual void loadSceneResource( std::string const &name, vl::TextResource &data ) = 0;

	virtual void loadPythonResource( std::string const &name, vl::TextResource &data ) = 0;

	virtual void loadOggResource( std::string const &name, vl::Resource &data ) = 0;

	virtual void loadMeshResource( std::string const &name, vl::Resource &data ) = 0;

	/// Resource path management
	virtual void addResourcePath( std::string const &resource_dir, bool recursive = true ) = 0;

	virtual std::vector<std::string> const &getResourcePaths( void ) const = 0;

};	// class ResourceManager

}	// namepsace vl

#endif // VL_RESOURCE_MANAGER_HPP
