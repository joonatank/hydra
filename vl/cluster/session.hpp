/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2014 Savant Simulators Oy
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-01
 *	@file cluster/session.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
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

	/// @brief Register object to synchronisation system
	/// Can be called from either master or a slave
	/// @param id either a valid id or ID_UNDEFINED if undefined will register a new object
	/// if valid will map already registered object
	void registerObject(vl::Distributed *obj, OBJ_TYPE type, uint64_t id)
	{
		assert(obj);
		// Already registered objects should not be here
		assert(obj->getID() == vl::ID_UNDEFINED);
		// Register object
		if( vl::ID_UNDEFINED == id )
		{
			_registerObject(obj, type);
		}
		else
		{
			_mapObject(obj, id);
		}
	}

	/// @brief master only overload
	void registerObject(vl::Distributed *obj, OBJ_TYPE type)
	{ registerObject(obj, type, vl::ID_UNDEFINED); }

	/// @brief slave only overload
	void registerObject(vl::Distributed *obj, uint64_t id)
	{ registerObject(obj, OBJ_INVALID, id); }

	/// @brief Remove registed object
	/// Can be called from either master or a slave
	void deregisterObject(vl::Distributed *obj)
	{
		assert(obj);
		assert(obj->getID() != vl::ID_UNDEFINED);
		
		// this could be optimized by starting from id
		DistributedObjectList::iterator iter 
			= std::find(_registered_objects.begin(), _registered_objects.end(), obj);
		// This function can be called from slaves, which do not have 
		// registered or destroyed object lists at all.
		if(iter != _registered_objects.end())
		{
			std::clog << "Destroying registered object " << obj->getID() << std::endl;
			_registered_objects.erase(iter);
			_destroyed_objects.push_back(obj->getID());
		}

		/// Check mapped objects for slave
		/// @todo the object needs to be either in mapped or registered (XOR)
		iter = std::find(_mapped_objects.begin(), _mapped_objects.end(), obj);
		if(iter != _mapped_objects.end())
		{
			std::clog << "Destroying mapped object " << obj->getID() << std::endl;
			_mapped_objects.erase(iter);
		}

		// Set the object as deregistered
		obj->registered(vl::ID_UNDEFINED);
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
	typedef std::vector<Distributed *> DistributedObjectList;
	typedef std::vector<uint64_t> IDList;

	CreatedObjectsList const &getNewObjects( void ) const
	{ return _new_objects; }

	void clearNewObjects( void )
	{ _new_objects.clear(); }

	IDList const &getDestroyedObjects(void) const
	{ return _destroyed_objects; }

	void clearDestroyedObjects(void)
	{ _destroyed_objects.clear(); }

	DistributedObjectList const &getRegistedObjects(void) const
	{ return _registered_objects; }

private :
	/// @brief Implementation for master side object registering
	void _registerObject(vl::Distributed *obj, OBJ_TYPE type)
	{
		_last_id++;
		obj->registered(_last_id);
		_new_objects.push_back( std::make_pair(type, obj) );
		_registered_objects.push_back(obj);
	}

	/// @brief Implementation for slave side object registering
	/// Maps the object to an ID
	void _mapObject(vl::Distributed *obj, uint64_t const id)
	{
		assert(obj);
		assert( obj->getID() == vl::ID_UNDEFINED );
		obj->registered(id);
		_mapped_objects.push_back(obj);
	}

protected :

	CreatedObjectsList _new_objects;

	/// @todo not used yet
	std::vector<uint64_t> _destroyed_objects;

	/// Registered data
	DistributedObjectList _registered_objects;
	uint64_t _last_id;

	/// Mapped data
	DistributedObjectList _mapped_objects;

};	// class Session

/// Global
inline std::ostream &
operator<<(std::ostream &os, vl::Session::DistributedObjectList const &list)
{
	os << std::dec << list.size() << " Distributed object list : " << std::endl;
	for(vl::Session::DistributedObjectList::const_iterator iter = list.begin(); iter != list.end(); ++iter)
	{
		os << std::hex << (*iter)->getID() << std::endl;
	}
	os << std::dec;

	return os;
}

}	// namespace vl

#endif // HYDRA_CLUSTER_SESSION_HPP
