#include "ogre_entity.hpp"

#include "ogre_scene_manager.hpp"

void
vl::ogre::Entity::load( vl::graph::SceneManager *sm ) 
{
	SceneManager *ogre_sm = (SceneManager *)sm;
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
