#include "ogre_entity.hpp"

#include "ogre_scene_manager.hpp"

vl::ogre::Entity::Entity( std::string const &name, vl::NamedValuePairList const &params )
	: vl::cl::Entity(name, params), _ogre_entity(0)
{}

Ogre::MovableObject *
vl::ogre::Entity::getNative( void )
{ return _ogre_entity; }

void
vl::ogre::Entity::load( void ) 
{
	boost::shared_ptr<SceneManager> ogre_sm
		= boost::dynamic_pointer_cast<SceneManager>( _manager.lock() );

	if( !ogre_sm )
	{ throw vl::bad_cast("vl::ogre::Entity::load"); }

	Ogre::NameValuePairList parm;
	vl::NamedValuePairList::iterator iter = _params.begin();
	for( ; iter != _params.end(); ++iter )
	{
		parm[iter->first] = iter->second;
		std::cout << "first = " <<  iter->first << " : second = "
			<< iter->second << std::endl;
	}
	
	if( !ogre_sm->getNative() )
	{ throw vl::null_pointer("vl::ogre::Entity::load"); }
	
	_ogre_entity = (Ogre::Entity *)( ogre_sm->getNative()->createMovableObject(
				getName(), "Entity", &parm) );

	if( !_ogre_entity )
	{ throw vl::null_pointer("vl::ogre::Entity::load"); }
}

// ---------- EntityFactory ----------
const std::string vl::ogre::EntityFactory::TYPENAME("Entity");

vl::graph::MovableObjectRefPtr
vl::ogre::EntityFactory::create( std::string const &name,
		vl::NamedValuePairList const &params )
{
	return vl::graph::MovableObjectRefPtr( new Entity( name, params ) );
}
