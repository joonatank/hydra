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
	Resource( void ) {}

	virtual ~Resource( void );

	/// copy constructors
	Resource( Resource const &other )
	{ copy( other ); }

	Resource &operator=( Resource const &other )
	{
		copy( other );
		return *this;
	}

	/// Virtual overrides
	/// Set the memory
	/// These will destroy the memory block already stored if any
	virtual void setRawMemory( vl::MemoryBlock const &block );
	virtual void setRawMemory( char *mem, size_t size );

	/// Release the memory block for user, the ownership of the memory block
	/// moves to the caller.
	virtual vl::MemoryBlock release( void );

	/// Copy data from another resource to this one.
	/// This will destroy any memory block already stored in this
	virtual void copy( Resource const &other );

protected :

	vl::MemoryBlock &getMemory( void )
	{ return _block; }

	vl::MemoryBlock _block;
};

}	// namespace eqOgre

#endif // EQ_OGRE_RESOURCE_HPP
