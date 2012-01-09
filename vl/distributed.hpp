/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011/10 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-01
 *	@file distributed.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef HYDRA_DISTRIBUTED_HPP
#define HYDRA_DISTRIBUTED_HPP

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

	uint64_t getDirty(void)
	{
		recaluclateDirties();
		return _dirtyBits;
	}

	/// @todo this should probably be protected?
	void setDirty( uint64_t const bits )
	{ _dirtyBits |= bits; }

	bool isDirty(void)
	{
		recaluclateDirties();
		return( _dirtyBits != DIRTY_NONE );
	}

	enum DirtyBits
	{
		DIRTY_NONE = 0,
		DIRTY_CUSTOM = 1,
		DIRTY_ALL = 0xFFFFFFFFFFFFFFFFull
	};

	/// @brief serializes the current dirties and the current modifications
	/// Pack and unpack needs to be symmetric in every way otherwise
	/// they will fail.
	void pack( cluster::ByteStream &msg ) const
	{ pack( msg, _dirtyBits ); }

	/// @brief serialize the object with some dirties
	void pack( cluster::ByteStream &msg, uint64_t const dirtyBits ) const
	{
		msg << dirtyBits;
		serialize( msg, dirtyBits );
	}

	void unpack( cluster::ByteStream &msg )
	{
		msg >> _dirtyBits;
		deserialize(msg, getDirty() );
	}

	uint64_t getID( void ) const
	{ return _id; }

	void registered( uint64_t const id )
	{
		_id = id;
		setDirty( DIRTY_ALL );
	}

	void clearDirty( void )
	{ _dirtyBits = DIRTY_NONE; }

private :
	/// @brief Recalculate member dirties if necessary
	virtual void recaluclateDirties(void) {}

	virtual void serialize( cluster::ByteStream &msg, const uint64_t dirtyBits ) const = 0;

	virtual void deserialize( cluster::ByteStream &msg, const uint64_t dirtyBits ) = 0;

	uint64_t _dirtyBits;
	uint64_t _id;

};	// class Distributed

}	// namespace vl

#endif	// HYDRA_DISTRIBUTED_HPP
