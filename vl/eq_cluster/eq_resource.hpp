/**	Joonatan Kuosa
 *	2010-11
 *
 *	Concrete implementation of vl::Resource
 *	Used to load resources into memory and then distribute them to the slave
 *	nodes and rendering threads.
 */

#ifndef EQ_OGRE_RESOURCE_HPP
#define EQ_OGRE_RESOURCE_HPP

#include "resource_manager.hpp"
#include <eq/net/object.h>

namespace eqOgre
{

/// Distributed resource class
/// Used to distribute memory blocks loaded with resource manager
class Resource : public vl::Resource
{
public :
	Resource( std::string const &name = std::string() );

	virtual ~Resource( void );

	/// copy constructors
	Resource( Resource const &other )
	{ copy( other ); }

	Resource &operator=( Resource const &other )
	{
		copy( other );
		return *this;
	}

	virtual std::string const &getName( void ) const
	{ return _name; }

	virtual void setName( std::string const &name )
	{ _name = name; }

	/// Virtual overrides
	/// Set the memory
	/// These will destroy the memory block already stored if any
	virtual void setRawMemory( vl::MemoryBlock const &block );
	virtual void setRawMemory( char *mem, size_t size );

	virtual vl::MemoryBlock const &getRawMemory( void ) const
	{ return _block; }

	/// Release the memory block for user, the ownership of the memory block
	/// moves to the caller.
	virtual vl::MemoryBlock release( void );

	/// Copy data from another resource to this one.
	/// This will destroy any memory block already stored in this
	virtual void copy( Resource const &other );

protected :

	vl::MemoryBlock &getMemory( void )
	{ return _block; }

	std::string _name;

	vl::MemoryBlock _block;
};

class ResourceManager : public vl::ResourceManager, public eq::net::Object
{
public :
	ResourceManager( void );

	virtual ~ResourceManager(void );

	/// Returns a modifiedle copy of the resource
	// TODO might be better to return references and have them copied in the
	// application code?
	Resource copyResource( std::string const &name ) const;

	std::vector<Resource> getSceneResources( void ) const;

	/// Add a resource to the list of will be loaded resources
	// TODO we should accommodate extra parameters for the resources, like scene
	// At the moment all resources use name based on the filename
	void addResource( std::string const &name );

	void removeResource( std::string const &name );

	/// Loads all resources to memory and distributes them to slave nodes
	void loadAllResources( void );

	// TODO add dirties
protected :
	virtual void getInstanceData( eq::net::DataOStream& os );
	virtual void applyInstanceData( eq::net::DataIStream& is );

	std::vector<Resource> _resources;
	std::vector<std::string> _waiting_for_loading;

};

eq::net::DataOStream &
operator<<( Resource &res, eq::net::DataOStream& os );


eq::net::DataIStream &
operator>>( Resource &res, eq::net::DataIStream& is );

}	// namespace eqOgre

#endif // EQ_OGRE_RESOURCE_HPP
