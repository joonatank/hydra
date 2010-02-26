#include "object_params.hpp"

eqOgre::object_param::object_param( OBJECT_TYPES type /*= INVALID*/,
		uint32_t id /*= EQ_ID_INVALID*/,
		bool created /*= false*/,
		eq::Object *ptr /*= 0*/,
		Ogre::String const &name /*= Ogre::String()*/,
		Ogre::String const &meshName /*= Ogre::String()*/,
		Ogre::String const &groupName /*= Ogre::String()*/
		)
	: _type(type), _id(id), _created(created), _ptr(ptr), _name(name),
	  _meshName(meshName),
	  _groupName(groupName)
{}

/*
eqOgre::entity_param::entity_param( OBJECT_TYPES type, 
		uint32_t id, bool created,
		Ogre::String const &name,
		Ogre::String const &meshName,
		Ogre::String const &groupName,
		)
	: object_param( type, id, created, ptr, name ),
	  _meshName(meshName),
	  _groupName(groupName)
{}
*/

eq::net::DataOStream &
eqOgre::operator<<( eq::net::DataOStream& os, eqOgre::object_param const &params )
{
	os << params._type << params._id << params._created << params._name; 

	if( params._type == ENTITY )
	{
		// We don't need this anymore we use only type flags.
		//eqOgre::entity_param const &ent =
		//	dynamic_cast<eqOgre::entity_param const &>(params);

		os << params._meshName << params._groupName;
	}

	return os;
}

eq::net::DataIStream &
eqOgre::operator>>( eq::net::DataIStream& is, eqOgre::object_param &params )
{
	is >> params._type >> params._id >> params._created >> params._name;

	if( params._type == ENTITY )
	{
		// Throws std::bad_cast if fails
		// TODO add some exception handling here or later,
		// now this will kill the whole application
		// (though it would indicate significant bug in the app).
		// We don't need this anymore, we use only type flags.
		//eqOgre::entity_param &ent = dynamic_cast<eqOgre::entity_param &>(params);

		is >> params._meshName >> params._groupName;
	}

	return is;
}
