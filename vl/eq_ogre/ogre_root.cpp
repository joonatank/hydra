#include "ogre_root.hpp"

vl::ogre::Root::Root( void )
	: vl::cl::Root(), _ogre_root(0), _primary(false)
{
	_ogre_root = Ogre::Root::getSingletonPtr();
	if( !_ogre_root )
	{
		_ogre_root = new Ogre::Root( "", "" );
		_primary = true;
	}
}

void
vl::ogre::Root::createRenderSystem( void )
{
	if( !_primary )
	{ return; }

	EQASSERT( _ogre_root );
#if defined(_DEBUG)
#ifdef VL_UNIX
	_ogre_root->loadPlugin("/usr/local/lib/OGRE/RenderSystem_GL_d");
#else
	_ogre_root->loadPlugin("RenderSystem_GL_d");
#endif
#else
#ifdef VL_UNIX
	_ogre_root->loadPlugin("/usr/local/lib/OGRE/RenderSystem_GL");
#else
	_ogre_root->loadPlugin("RenderSystem_GL");
#endif
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
		.addResourceLocation( "resources", "FileSystem", "General" );
}

vl::graph::RenderWindow *
vl::ogre::Root::createWindow( std::string const &name, unsigned int width,
		unsigned int height, vl::NamedValuePairList const &params )
{
	if( !_ogre_root )
	{ return 0; }

	static int n_windows = 0;
	static boost::mutex window_mutex;
			
	// TODO Add mutex
	boost::mutex::scoped_lock lock(window_mutex);

	Ogre::NameValuePairList misc;
	vl::NamedValuePairList::const_iterator iter = params.begin();
	for( ; iter != params.end(); ++iter )
	{ misc[iter->first] = iter->second; }

	std::stringstream ss( std::stringstream::in | std::stringstream::out );
	ss << name << "-" << n_windows;
	Ogre::RenderWindow *win =
		_ogre_root->createRenderWindow( ss.str(), width, height, false, &misc );
	++n_windows;

	if( !_ogre_root->isInitialised() )
	{ init(); }

	return new vl::ogre::RenderWindow( win );
}

