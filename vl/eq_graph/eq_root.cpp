#include "eq_root.hpp"

#include <eq/net/session.h>

#include <cstdlib>

#include "base/filesystem.hpp"

vl::cl::Root::Root( void )
	: _scene_managers()
{
	// TODO CMake should set variable VL_SOURCE_DIR when configuring
	// TODO should also be able to use VL_INSTALL_DIR for later use with installer
	char *dir = ::getenv( "VL_DIR" );
	if( dir )
	{
		fs::path path(dir);
		if( fs::exists( path ) )
		{ _base_dir = dir; }
		else
		{
			// Try to find based on the current VL_DIR
			path = find_parent_dir( "data", path );
			if( !fs::exists( path ) )
			{
				path = find_parent_dir( "data", fs::current_path() );
			}
			path.remove_filename();
			_base_dir = path.file_string();
		}
	}
	else
	{
		// Find path based on current directory
		fs::path path = find_parent_dir( "data", fs::current_path());
		path.remove_filename();
		_base_dir = path.file_string();
	}
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
