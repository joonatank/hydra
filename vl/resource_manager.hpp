/**
 *	Copyright (c) 2010 - 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-11
 *	@file resource_manager.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */


/**	
 *	@date 2011-05 Changed from Abstract to Concrete (removed the distributed version)
 *
 *	Class to load resources
 *	Holds a number of resource paths from where the resources are searched
 *
 *	Loads ascii file resources to std::strings
 *	Loads binary resources to memory
 */

#ifndef HYDRA_RESOURCE_MANAGER_HPP
#define HYDRA_RESOURCE_MANAGER_HPP

#include <vector>
#include <string>

#include "resource.hpp"

namespace vl
{

/// @class ResourceManager
class ResourceManager
{
public :
	ResourceManager(void);

	~ResourceManager(void);

	/**	@brief Find the resource with name from the search paths
	 *	If the resource is found the full path to it is stored in path
	 *
	 *	@return true if resource was found, false otherwise
	 *	@throw nothing
	 */
	bool findResource(std::string const &name, std::string &path) const;

	/**	@brief Loads a resource to the provided reference
	 *	@throw vl::resource_not_found if the resource by the name was not found
	 */
	void loadResource(std::string const &name, vl::Resource &data);

	void loadSceneResource(std::string const &name, vl::TextResource &data);

	void loadPythonResource(std::string const &name, vl::TextResource &data);

	void loadOggResource(std::string const &name, vl::Resource &data);

	void loadMeshResource(std::string const &name, vl::Resource &data);

	void loadRecording(std::string const &name, vl::Resource &data);

	/// Resource path management
	
	/**	@brief Add a resource path to the resource search paths
	 *	@param resource_dir needs to be a valid filesystem directory
	 *	@param recursive true if you want all the subdirectories added too
	 *
	 *	@throw if the resource_dir does not exist or is not a directory
	 */
	void addResourcePath(std::string const &resource_dir, bool recursive = true);

	void removeResourcePath(std::string const &resource_dir);

	/**	@brief Get the current resource search paths
	 *	@return current search paths
	 *
	 *	Always returns the current search paths, if there is none will return
	 *	an empty vector.
	 *
	 *	@throw nothing
	 */
	std::vector<std::string> const &getResourcePaths(void) const;

private :
	/// Really load the resource
	void _loadResource( std::string const &name,
						std::string const &path,
						vl::Resource &resource ) const;

	/// Helper functions for manipulating resource files
	std::string _getFileName( std::string const &name, std::string const &extension );
	std::string _stripExtension( std::string const &name, std::string const &extension );

	std::vector<std::string> _search_paths;

};	// class ResourceManager

}	// namepsace vl

#endif // HYDRA_RESOURCE_MANAGER_HPP
