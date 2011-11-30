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

enum OBJ_TYPE
{
	OBJ_INVALID,
	OBJ_PLAYER,
	OBJ_GUI,				// GUI object
	OBJ_GUI_WINDOW,			// Genera GUI window
	OBJ_GUI_CONSOLE,		// GUI Console window
	OBJ_GUI_EDITOR,			// GUI editor window
	OBJ_GUI_SCENE_GRAPH_EDITOR,	// GUI scene graph editor
	OBJ_GUI_MATERIAL_EDITOR,	// GUI material editor
	OBJ_GUI_SCRIPT_EDITOR,	// GUI material editor
	OBJ_SCENE_MANAGER,
	OBJ_SCENE_NODE,
	OBJ_ENTITY,
	OBJ_LIGHT,
	OBJ_CAMERA,
	OBJ_MOVABLE_TEXT,
	OBJ_RAY_OBJECT,
};

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

	/// Registered data
	std::vector<vl::Distributed *> _registered_objects;
	uint64_t _last_id;

	/// Mapped data
	std::vector<vl::Distributed *> _mapped_objects;

};	// class Session

}	// namespace vl

#endif // VL_CLUSTER_SESSION_HPP