/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2011-01
 */

#ifndef VL_DISTRIBUTED_HPP
#define VL_DISTRIBUTED_HPP

#include "cluster/message.hpp"

namespace vl
{

const uint64_t ID_UNDEFINED = 0;

class Distributed
{
public :
	Distributed( void )
		: _dirtyBits(0), _id( ID_UNDEFINED )
	{}

	virtual ~Distributed( void ) {}

	uint64_t getDirty( void ) const
	{ return _dirtyBits; }

	void setDirty( uint64_t const bits )
	{ _dirtyBits |= bits; }

	bool isDirty( void ) const
	{ return( _dirtyBits != DIRTY_NONE ); }

	enum DirtyBits
	{
		DIRTY_NONE = 0,
		DIRTY_CUSTOM = 1,
		DIRTY_ALL = 0xFFFFFFFFFFFFFFFFull
	};

	/// Pack and unpack needs to be symmetric in every way otherwise
	/// they will fail.
	void pack( cluster::Message &msg )
	{ pack( msg, _dirtyBits ); }

	void pack( cluster::Message &msg, uint64_t const dirtyBits )
	{
		msg.write( dirtyBits );
		serialize( msg, getDirty() );
		clearDirty();
	}

	void unpack( cluster::Message &msg )
	{
		msg.read(_dirtyBits);
		deserialize(msg, getDirty() );
		clearDirty();
	}

	uint64_t getID( void ) const
	{ return _id; }

	void registered( uint64_t const id )
	{
		_id = id;
		setDirty( DIRTY_ALL );
	}

private :
	virtual void serialize( cluster::Message &msg, const uint64_t dirtyBits ) = 0;

	virtual void deserialize( cluster::Message &msg, const uint64_t dirtyBits ) = 0;

	void clearDirty( void )
	{ _dirtyBits = DIRTY_NONE; }

	uint64_t _dirtyBits;
	uint64_t _id;

};	// class Distributed

}	// namespace vl

#endif	// VL_DISTRIBUTED_HPP