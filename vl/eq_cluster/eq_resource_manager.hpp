/**	Joonatan Kuosa
 *	2010-11
 *
 *	Concrete implementation of vl::ResourceManager
 *	Interface to load resources into memory.
 *	All the resources loaded are still stored in the resource manager. So that
 *	they can be distributed to the slave nodes (rendering threads).
 */

#ifndef EQOGRE_RESOURCE_MANAGER_HPP
#define EQOGRE_RESOURCE_MANAGER_HPP

#include <eq/net/object.h>

// Base class
#include "resource_manager.hpp"

namespace eqOgre
{

class ResourceManager : public vl::ResourceManager, public eq::net::Object
{
public :
	ResourceManager( void );

	virtual ~ResourceManager(void );

	// TODO this should be removed as soon as we have a decent system
	// Settings should distribute the used scene files and this sould distribute
	// the content of those files
	std::vector<vl::TextResource> const &getSceneResources( void ) const;

	/// Add a resource to the list of will be loaded resources
	/// TODO not implemented
	virtual void addResource( std::string const &name );

	/// TODO not implemented
	virtual void removeResource( std::string const &name );

	/**	Loads all resources to memory
	 *	If resource is loaded and it's a type supposed to be distributed
	 *	then adds it to the distributed stack
	 *
	 *	TODO not implemented
	 */
	virtual void loadAllResources( void );

	/**	Find the resource with name from the search paths
	 *	If the resource is found the full path to it is stored in path
	 *
	 *	Returns : true if resource was found, false otherwise
	 *
	 *	Throws : nothing
	 */
	virtual bool findResource( std::string const &name, std::string &path ) const;

	/**	Loads a resource to the provided reference
	 *	If resource is loaded and it's a type supposed to be distributed
	 *	then adds it to the distributed stack
	 *
	 *	Throws vl::resource_not_found if the resource by the name was not found
	 */
	virtual void loadResource( std::string const &name, vl::Resource &data );

	virtual void loadSceneResource( std::string const &name, vl::TextResource &data );

	/**	Add a resource path to the resource search paths
	 *	Parameters : resource_dir needs to be a valid filesystem directory
	 *				 recursive true if you want all the subdirectories added too
	 *
	 *	Throws if the resource_dir does not exist or is not a directory
	 */
	virtual void addResourcePath( std::string const &resource_dir, bool recursive = true );

	/**	Get the current resource search paths
	 *
	 *	Returns always the current search paths, if there is none will return
	 *	an empty vector.
	 *
	 *	Can not fail.
	 */
	virtual std::vector<std::string> const &getResourcePaths( void ) const;


	// TODO add dirties


protected :

	/// Find and copy the resource from the stack
	bool _findLoadedResource( std::string const &res_name, vl::Resource &resource ) const;

	/// Really load the resource
	void _loadResource( std::string const &path, vl::Resource &resource ) const;

	virtual void getInstanceData( eq::net::DataOStream& os );
	virtual void applyInstanceData( eq::net::DataIStream& is );

// 	std::vector<vl::Resource> _resources;

	std::vector<vl::TextResource> _scenes;

// 	std::vector<std::string> _waiting_for_loading;

	std::vector<std::string> _search_paths;

};	// class ResourceManager

eq::net::DataOStream &
operator<<( vl::Resource &res, eq::net::DataOStream& os );


eq::net::DataIStream &
operator>>( vl::Resource &res, eq::net::DataIStream& is );

}	// namespace eqOgre

#endif // EQOGRE_RESOURCE_MANAGER_HPP
