#include "eq_root.hpp"

#include <eq/net/session.h>

vl::cl::Root::Root( void )
	: _scene_manager(0)
{}

vl::graph::SceneManager *
vl::cl::Root::getSceneManager( std::string const &name )
{
	if( !_scene_manager )
	{
		_scene_manager = _createSceneManager( name );
		// TODO we need to check that we are registered here
		if( isMaster() )
		{
			getSession()->registerObject( _scene_manager );
			setDirty( DIRTY_SCENE_MANAGER );
		}
		_scene_manager->finalize();
	}
	return _scene_manager;
}

void
vl::cl::Root::serialize( eq::net::DataOStream& os,
		const uint64_t dirtyBits )
{
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
}

void
vl::cl::Root::deserialize( eq::net::DataIStream& is,
		  const uint64_t dirtyBits )
{
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
}
