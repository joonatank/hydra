#ifndef EQ_OGRE_OBJECT_PARAMS_HPP
#define EQ_OGRE_OBJECT_PARAMS_HPP

#include <OGRE/OgreString.h>

#include <eq/net/dataOStream.h>
#include <eq/net/dataIStream.h>

#include <eq/net/object.h>
#include <eq/client/object.h>

#include "utility.hpp"

namespace eqOgre
{
	enum OBJECT_TYPES
	{
		INVALID = 0,
		SCENE_MANAGER,
		CAMERA,
		LIGHT,
		SCENE_NODE,
		ENTITY
	};

	class object_param
	{
		public :
			object_param( OBJECT_TYPES type = INVALID,
					uint32_t id = EQ_ID_INVALID, bool created = false,
					eq::Object *ptr = 0,
					Ogre::String const &name = Ogre::String(),
					Ogre::String const &meshName = Ogre::String(),
					Ogre::String const &groupName = Ogre::String()
					);

			virtual ~object_param( void ) {}

			OBJECT_TYPES _type;
			uint32_t _id;
			bool _created;
			eq::Object *_ptr;
			Ogre::String _name;

			// Entity specific params
			// We might use paramList for this also, TODO explore the possibility
			// We can not use inheritance if we want to transmit these
			// mostly because we can not delegate the task of creating the object
			// to someone who does not read from the stream.
			// Using dynamic container like Ogre::ParamList would solve this
			// problem nicely.
			Ogre::String _meshName;
			Ogre::String _groupName;

	};	// class object_param

	/*
	class entity_param : public object_param
	{
		public :
			entity_param( OBJECT_TYPES type = INVALID,
					uint32_t id = EQ_ID_INVALID, bool created = false,
					eq::Object *ptr = 0,
					Ogre::String const &name = Ogre::String(),
					Ogre::String const &meshName = Ogre::String(),
					Ogre::String const &groupName = Ogre::String()
					);

			virtual ~entity_param( void ) {}

			Ogre::String _meshName;
			Ogre::String _groupName;
	};	// class entity_params
	*/

	typedef std::vector<object_param> ObjectParamVec;

	eq::net::DataOStream &
	operator<<( eq::net::DataOStream& os,
			eqOgre::object_param const& params );

	eq::net::DataIStream &
	operator>>( eq::net::DataIStream& is,
			eqOgre::object_param& params );

	/*
	template<>
	eq::net::DataIStream &
	operator>>( eq::net::DataIStream& is, std::vector<object_params> &vec )
	{
		size_t size;
		is >> size;
		std::vector<object_params>::iterator iter;
		for( iter = vec.begin(); iter != vec.end(); ++iter )
		{
			
		}

	}
	*/
}	// namespace eqOgre

#endif
