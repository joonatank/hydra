/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2012 Savant Simulators Oy
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-01
 *	@file cluster/session.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

#ifndef HYDRA_CLUSTER_SESSION_HPP
#define HYDRA_CLUSTER_SESSION_HPP

#include "object_types.hpp"
#include "distributed.hpp"

#include <stdint.h>

#include <vector>
// Necessary for memcpy
#include <cstring>

#include <iostream>

#include <assert.h>

namespace vl
{

class Session
{
public :
	Session( void )
		: _last_id(0)
	{}

	virtual ~Session( void ) {}

	/// Register object to synchronisation system
	/// @param id either a valid id or ID_UNDEFINED if undefined will register a new object
	/// if valid will map already registered object
	void registerObject( vl::Distributed *obj, OBJ_TYPE type, uint64_t id = vl::ID_UNDEFINED )
	{
		assert(obj);
		// Already registered objects should not be here
		assert(obj->getID() == vl::ID_UNDEFINED);
		// Register object
		if( vl::ID_UNDEFINED == id )
		{
			_last_id++;
			obj->registered(_last_id);
			_new_objects.push_back( std::make_pair(type, obj) );
			_registered_objects.push_back(obj);
		}
		else
		{
			mapObject(obj, id);
		}
	}

	void deregisterObject(vl::Distributed *obj)
	{
		assert(obj);
		assert(obj->getID() != vl::ID_UNDEFINED);
		
		// this could be optimized by starting from id
		std::vector<vl::Distributed *>::iterator iter 
			= std::find(_registered_objects.begin(), _registered_objects.end(), obj);
		assert(iter != _registered_objects.end());
		_registered_objects.erase(iter);
		_destroyed_objects.push_back(obj->getID());

		// Set the object as deregistered
		obj->registered(vl::ID_UNDEFINED);
	}

	void mapObject( vl::Distributed *obj, uint64_t const id )
	{
		assert(obj);
		assert( obj->getID() == vl::ID_UNDEFINED );
		obj->registered(id);
		_mapped_objects.push_back(obj);
	}

	vl::Distributed *findMappedObject( uint64_t const id )
	{
		std::vector<vl::Distributed *>::iterator iter;
		for( iter = _mapped_objects.begin(); iter != _mapped_objects.end();
			++iter )
		{
			if( (*iter)->getID() == id )
			{ return *iter; }
		}

		return 0;
	}

	typedef std::vector< std::pair<OBJ_TYPE, Distributed *> > CreatedObjectsList;

	CreatedObjectsList const &getNewObjects( void ) const
	{
		return _new_objects;
	}

	void clearNewObjects( void )
	{ _new_objects.clear(); }

protected :

	CreatedObjectsList _new_objects;

	std::vector<uint64_t> _destroyed_objects;

	/// Registered data
	std::vector<vl::Distributed *> _registered_objects;
	uint64_t _last_id;

	/// Mapped data
	std::vector<vl::Distributed *> _mapped_objects;

};	// class Session

}	// namespace vl

#endif // HYDRA_CLUSTER_SESSION_HPP
