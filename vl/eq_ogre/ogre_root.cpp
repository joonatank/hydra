#include "ogre_root.hpp"

#include <OGRE/OgreConfigFile.h>

vl::ogre::Root::Root( void )
	: vl::cl::Root(), _ogre_root(0), _primary(false)
{
	_ogre_root = Ogre::Root::getSingletonPtr();
	if( !_ogre_root )
	{
		std::string plugins;
#if defined _DEBUG
		plugins = "plugins_d.cfg";
#else
		plugins = "plugins.cfg";
#endif
		std::string plugin_path = _base_dir + std::string("/data/") + plugins;
		_ogre_root = new Ogre::Root( plugin_path , "" );
		_primary = true;

		std::string msg( "plugin path = " + plugin_path );
		Ogre::LogManager::getSingleton().logMessage( msg );
	}
}

vl::ogre::Root::~Root( void )
{
	// FIXME this can not destroy ogre root if we have multiple
	// Roots pointing to same ogre singleton.
	if( _primary )
	{ delete _ogre_root; }
}

void
vl::ogre::Root::createRenderSystem( void )
{
	if( !_primary )
	{ return; }

	EQASSERT( _ogre_root );

	// We only support OpenGL rasterizer
	Ogre::RenderSystem *rast
		= _ogre_root->getRenderSystemByName( "OpenGL Rendering Subsystem" );
	if( !rast )
	{ throw vl::exception( "No OpenGL rendering system plugin found",
			"vl::ogre::Root::createRenderSystem" ); }
	else
	{ _ogre_root->setRenderSystem( rast ); }
}

void
vl::ogre::Root::init( void )
{
	_ogre_root->initialise( false );

	setupResources();
	loadResources();
}

/// Method which will define the source of resources (other than current folder)
void
vl::ogre::Root::setupResources(void)
{
	std::string resource_file( _base_dir + std::string("/data/") );
#if( OGRE_DEBUG_MODE == 1 )
	resource_file += "resources_d.cfg";
#else
	resource_file += "resources.cfg";
#endif

	std::string msg( "Using resource file = " + resource_file );
	Ogre::LogManager::getSingleton().logMessage( msg );

	// Load resource paths from config file
	Ogre::ConfigFile cf;
	cf.load( resource_file );

	// Go through all sections & settings in the file
	Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

	std::string secName, typeName, archName;
	while (seci.hasMoreElements())
	{
		secName = seci.peekNextKey();
		Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
		Ogre::ConfigFile::SettingsMultiMap::iterator i;
		for (i = settings->begin(); i != settings->end(); ++i)
		{
			typeName = i->first;
			archName = i->second;
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
			// OS X does not set the working directory relative to the app,
			// In order to make things portable on OS X we need to provide
			// the loading with it's own bundle path location
			Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
				std::string(macBundlePath() + "/" + archName), typeName, secName);
#else
			Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
					archName, typeName, secName);
#endif
		}
	}
}

void
vl::ogre::Root::loadResources(void)
{
	// Initialise, parse scripts etc
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}

vl::graph::RenderWindowRefPtr
vl::ogre::Root::createWindow( std::string const &name, unsigned int width,
		unsigned int height, vl::NamedValuePairList const &params )
{
	if( !_ogre_root )
	{ return vl::graph::RenderWindowRefPtr(); }

	static int n_windows = 0;
			
	Ogre::NameValuePairList misc;
	vl::NamedValuePairList::const_iterator iter = params.begin();
	for( ; iter != params.end(); ++iter )
	{ misc[iter->first] = iter->second; }

	std::stringstream ss( std::stringstream::in | std::stringstream::out );
	ss << name << "-" << n_windows;
	Ogre::RenderWindow *win =
		_ogre_root->createRenderWindow( ss.str(), width, height, false, &misc );
	++n_windows;

	// Initialise the rendering system and load resources automatically when
	// first window is created.
	if( !_ogre_root->isInitialised() )
	{ init(); }

	return vl::graph::RenderWindowRefPtr( new vl::ogre::RenderWindow( win ) );
}

