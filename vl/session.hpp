/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2011-01
 */

#ifndef VL_CLUSTER_SESSION_HPP
#define VL_CLUSTER_SESSION_HPP

#include <stdint.h>

#include <vector>
// Necessary for memcpy
#include <cstring>

#include <iostream>
#include <distributed.hpp>

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

	/// Register object to our own synchronisation system
	void registerObject( vl::Distributed *obj )
	{
		assert(obj);
		assert( obj->getID() == vl::ID_UNDEFINED );
		_last_id++;
		obj->registered(_last_id);
		_registered_objects.push_back(obj);
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

protected :

	/// Registered data
	std::vector<vl::Distributed *> _registered_objects;
	uint64_t _last_id;

	/// Mapped data
	std::vector<vl::Distributed *> _mapped_objects;

};	// class Session

}	// namespace vl

#endif // VL_CLUSTER_SESSION_HPP