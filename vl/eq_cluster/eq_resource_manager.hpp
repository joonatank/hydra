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

#include "distributed.hpp"

// Base class
#include "resource_manager.hpp"

namespace eqOgre
{

class ResourceManager : public vl::ResourceManager, public vl::Distributed
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

	virtual void loadPythonResource( std::string const &name, vl::TextResource &data );

	virtual void loadOggResource( std::string const &name, vl::Resource &data );

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

	enum DirtyBits
	{
		DIRTY_PATHS = vl::Distributed::DIRTY_CUSTOM << 0,
		DIRTY_SCENES = vl::Distributed::DIRTY_CUSTOM << 1,
		DIRTY_CUSTOM = vl::Distributed::DIRTY_CUSTOM << 3
	};

protected :

	/// Find and copy the resource from the stack
	bool _findLoadedResource( std::string const &res_name, vl::Resource &resource ) const;

	/// Really load the resource
	void _loadResource( std::string const &name,
						std::string const &path,
						vl::Resource &resource ) const;

	/// Helper functions for manipulating resource files
	std::string _getFileName( std::string const &name, std::string const &extension );
	std::string _stripExtension( std::string const &name, std::string const &extension );

	/// Overload Distributed virtuals
	virtual void serialize( vl::cluster::Message &msg, const uint64_t dirtyBits );
	virtual void deserialize( vl::cluster::Message &msg, const uint64_t dirtyBits );

	std::vector<vl::TextResource> _scenes;

	std::vector<vl::TextResource> _python_scripts;

	std::vector<vl::Resource> _ogg_sounds;

	std::vector<std::string> _search_paths;

};	// class ResourceManager

}	// namespace eqOgre

namespace vl
{

namespace cluster
{

template<>
inline
vl::cluster::Message &operator<<( vl::cluster::Message &msg, vl::TextResource const &res )
{
// 	std::cout << "operator<< TextResource" << std::endl;
	// TODO add name
	msg << res.size();
	msg.write( res.get(), res.size() );

// 	std::cout << "operator<< TextResource done" << std::endl;
	return msg;
}

template<>
inline
vl::cluster::Message &operator>>( vl::cluster::Message &msg, vl::TextResource &res )
{
// 	std::cout << "operator>> TextResource" << std::endl;
	size_t size;
	msg >> size;
	res.resize(size);
	msg.read( res.get(), size );

	return msg;
}

}	// namespace cluster

}	// namespace vl

#endif // EQOGRE_RESOURCE_MANAGER_HPP
