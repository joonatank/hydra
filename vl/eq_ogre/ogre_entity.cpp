#include "ogre_entity.hpp"

#include "ogre_scene_manager.hpp"

void
vl::ogre::Entity::load( vl::graph::SceneManagerRefPtr sm ) 
{
	boost::shared_ptr<SceneManager> ogre_sm
		= boost::dynamic_pointer_cast<SceneManager>( sm );

	if( !ogre_sm )
	{ throw vl::null_pointer("vl::ogre::Entity::load"); }

	// TODO this can be used for all MovableObjects not just meshes
	Ogre::NameValuePairList parm;
	vl::NamedValuePairList::iterator iter = _params.begin();
	for( ; iter != _params.end(); ++iter )
	{
		parm[iter->first] = iter->second;
		std::cout << "first = " <<  iter->first << " : second = "
			<< iter->second << std::endl;
	}
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
