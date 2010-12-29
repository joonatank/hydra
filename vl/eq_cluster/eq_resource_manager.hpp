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

#include <co/object.h>

// Base class
#include "resource_manager.hpp"

#include "eq_resource.hpp"

namespace eqOgre
{

class ResourceManager : public vl::ResourceManager, public co::Object
{
public :
	ResourceManager( void );

	virtual ~ResourceManager(void );

	/// Returns a modifiedle copy of the resource
	// TODO might be better to return references and have them copied in the
	// application code?
	// Problems with references are that if you use them to copy the object
	// if it's a base class reference you slice the data when copying.
	Resource copyResource( std::string const &name ) const;

	// TODO this should be removed as soon as we have a decent system
	// Settings should distribute the used scene files and this sould distribute
	// the content of those files
	std::vector<Resource> getSceneResources( void ) const;

	/// Add a resource to the list of will be loaded resources
	virtual void addResource( std::string const &name );

	virtual void removeResource( std::string const &name );

	/**	Loads all resources to memory
	 *	If resource is loaded and it's a type supposed to be distributed
	 *	then adds it to the distributed stack
	 *
	 *	Return value : true if all resources were loaded, false otherwise
	 */
	virtual bool loadAllResources( void );

	virtual bool findResource( std::string const &name, std::string &path ) const;

	/**	Loads a resource to the provided reference
	 *	If resource is loaded and it's a type supposed to be distributed
	 *	then adds it to the distributed stack
	 *
	 *	Return value : true if resource was loaded, false otherwise
	 */
	virtual bool loadResource( std::string const &name, vl::Resource &data );

	/// Manage resource paths
	virtual bool addResourcePath( std::string const &resource_dir, bool recursive = true );

	virtual std::vector<std::string> const &getResourcePaths( void ) const;


	// TODO add dirties


protected :

	/// Find and copy the resource from the stack
	bool _findResource( std::string const &res_name, vl::Resource &resource ) const;

	virtual void getInstanceData( co::DataOStream& os );
	virtual void applyInstanceData( co::DataIStream& is );

	std::vector<Resource> _resources;

	std::vector<std::string> _waiting_for_loading;

	std::vector<std::string> _search_paths;

};	// class ResourceManager

co::DataOStream &
operator<<( Resource &res, co::DataOStream& os );


co::DataIStream &
operator>>( Resource &res, co::DataIStream& is );

}	// namespace eqOgre

#endif // EQOGRE_RESOURCE_MANAGER_HPP
