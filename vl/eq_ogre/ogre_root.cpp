#include "ogre_root.hpp"

vl::ogre::Root::Root( void )
	: vl::cl::Root(), _ogre_root(0)
{
	_ogre_root = new Ogre::Root( "", "" );
}

void
vl::ogre::Root::createRenderSystem( void )
{
	EQASSERT( _ogre_root );
#if defined(_DEBUG)
	_ogre_root->loadPlugin("RenderSystem_GL_d");
#else
	_ogre_root->loadPlugin("RenderSystem_GL");
#endif

	Ogre::RenderSystemList::iterator r_it;
	Ogre::RenderSystemList renderSystems
		= _ogre_root->getAvailableRenderers();
	EQASSERT( !renderSystems.empty() );
	r_it = renderSystems.begin();
	_ogre_root->setRenderSystem(*r_it);
}

void
vl::ogre::Root::init( void )
{
	_ogre_root->initialise( false );

	Ogre::ResourceGroupManager::getSingleton()
		.addResourceLocation( "resources",
				"FileSystem", "General" );
}

vl::graph::RenderWindow *
vl::ogre::Root::createWindow(
		std::string const &name, unsigned int width,
		unsigned int height,
		vl::NamedValuePairList const &params )
{
	if( _ogre_root )
	{
		Ogre::NameValuePairList misc;
		vl::NamedValuePairList::const_iterator iter 
			= params.begin();
		for( ; iter != params.end(); ++iter )
		{ misc[iter->first] = iter->second; }

		Ogre::RenderWindow *win =
			_ogre_root->createRenderWindow( name, width, height,
				false, &misc );
		return new vl::ogre::RenderWindow( win );
	}
	return 0;
}

