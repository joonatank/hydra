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

	Resource( vl::Resource const &other )
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
	virtual void copy( vl::Resource const &other );

protected :

	std::string _name;

	vl::MemoryBlock _block;
};

}	// namespace eqOgre

#endif // EQ_OGRE_RESOURCE_HPP
