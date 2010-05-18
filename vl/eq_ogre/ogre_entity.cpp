#include "ogre_entity.hpp"

#include "ogre_scene_manager.hpp"

void
vl::ogre::Entity::setManager(vl::graph::SceneManagerRefPtr man)
{
	vl::cl::Entity::setManager(man);

	boost::shared_ptr<SceneManager> og_man =
		boost::dynamic_pointer_cast<vl::ogre::SceneManager>( man );
	if( !og_man )
	{
		// TODO replace with cast failed
		BOOST_THROW_EXCEPTION( vl::null_pointer() );
//		throw vl::null_pointer( "vl::ogre::Entity::Entity" );
	}

	load();
}

void
vl::ogre::Entity::load( void )
{
	boost::shared_ptr<SceneManager> ogre_sm
		= boost::dynamic_pointer_cast<SceneManager>( _manager.lock() );

	if( !ogre_sm )
	{
		// TODO replace with cast failed
		BOOST_THROW_EXCEPTION( vl::null_pointer() );
	//	throw vl::null_pointer("vl::ogre::Entity::load");
	}

	// TODO this can be used for all MovableObjects not just meshes
	Ogre::NameValuePairList parm;
	vl::NamedValuePairList::iterator iter = _params.begin();
	for( ; iter != _params.end(); ++iter )
	{
		parm[iter->first] = iter->second;
//		std::cout << "first = " <<  iter->first << " : second = "
//			<< iter->second << std::endl;
	}
	
	if( !ogre_sm->getNative() )
	{
		// TODO replace with no native object
		BOOST_THROW_EXCEPTION( vl::null_pointer() );
//		throw vl::null_pointer("vl::ogre::Entity::load");
	}
	
	_ogre_entity = (Ogre::Entity *)( ogre_sm->getNative()->createMovableObject(
				getName(), "Entity", &parm) );

	if( !_ogre_entity )
	{
		// TODO replace with no native object
		BOOST_THROW_EXCEPTION( vl::null_pointer() );
//		throw vl::null_pointer("vl::ogre::Entity::load");
	}
}

void
vl::ogre::Entity::setCastShadows(bool castShadows)
{
	vl::cl::Entity::setCastShadows(castShadows);
	if( _ogre_entity )
	{ _ogre_entity->setCastShadows(castShadows); }
}

void
vl::ogre::Entity::setMaterialName(const std::string& materialFile)
{
	vl::cl::Entity::setMaterialName(materialFile);
	if( _ogre_entity )
	{ _ogre_entity->setMaterialName(materialFile); }
}

// ---------- EntityFactory ----------
const std::string vl::ogre::EntityFactory::TYPENAME("Entity");

vl::graph::MovableObjectRefPtr
vl::ogre::EntityFactory::create( std::string const &name,
		vl::NamedValuePairList const &params )
{
	return vl::graph::MovableObjectRefPtr( new Entity( name, params ) );
}
