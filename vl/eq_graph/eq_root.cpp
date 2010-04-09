#include "eq_root.hpp"

#include <eq/net/session.h>

#include <cstdlib>

vl::cl::Root::Root( void )
	: _scene_managers()
{
	char *dir = ::getenv( "VL_DIR" );
	if( dir )
	{ _base_dir = dir; }
}

vl::graph::SceneManagerRefPtr
vl::cl::Root::getSceneManager( std::string const &name )
{
	if( name.empty() )
	{ throw vl::empty_param("vl::cl::Root::getSceneManager"); }

	std::vector<vl::graph::SceneManagerRefPtr>::iterator iter
		= _scene_managers.begin();
	for( ; iter != _scene_managers.end(); ++iter )
	{
		if( (*iter)->getName() == name )
		{ return *iter; }
	}

	return vl::graph::SceneManagerRefPtr();
}

vl::graph::SceneManagerRefPtr
vl::cl::Root::createSceneManager( std::string const &name )
{
	if( name.empty() )
	{ throw vl::empty_param("vl::cl::Root::createSceneManager"); }

	vl::graph::SceneManagerRefPtr man = _createSceneManager( name );
	_scene_managers.push_back( man );

	return man;
}

void
vl::cl::Root::destroySceneManager( std::string const &name )
{
	if( name.empty() )
	{ throw vl::empty_param("vl::cl::Root::getSceneManager"); }

	std::vector<vl::graph::SceneManagerRefPtr>::iterator iter;
	for( iter = _scene_managers.begin(); iter != _scene_managers.end(); ++iter )
	{
		if( (*iter)->getName() == name )
		{
			_scene_managers.erase( iter );
			return;
		}
	}

	throw vl::no_object("vl::cl::Root::getSceneManager");
}

void
vl::cl::Root::serialize( eq::net::DataOStream& os,
		const uint64_t dirtyBits )
{
	eq::Object::serialize( os, dirtyBits );
	/*
	if( DIRTY_SCENE_MANAGER & dirtyBits )
	{
		if( _scene_manager )
		{
			std::cout << "Serializing SceneManager" << std::endl;
			os << _scene_manager->getID();
			os << _scene_manager->getName();
		}
		else
		{
			os << EQ_ID_INVALID;
		}
	}
	*/
}

void
vl::cl::Root::deserialize( eq::net::DataIStream& is,
		  const uint64_t dirtyBits )
{
	eq::Object::deserialize( is, dirtyBits );
	/*
	if( DIRTY_SCENE_MANAGER & dirtyBits )
	{
		uint32_t id;
		is >> id;
		if( id != EQ_ID_INVALID )
		{
			// For now we only support one scene_manager
			std::string name;
			is >> name;
			delete _scene_manager;
			_scene_manager = 0;
			_scene_manager = _createSceneManager( name );
			if( _scene_manager )
			{
				_scene_manager->finalize();
				std::cout << "Mapping SceneManager" << std::endl;
				getSession()->mapObject( _scene_manager, id );
			}
		}
	}
	*/
}
