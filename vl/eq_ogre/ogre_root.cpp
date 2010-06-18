
// Interface include
#include "ogre_root.hpp"

// library includes
#include "base/exceptions.hpp"

// Ogre includes
#include <OGRE/OgreConfigFile.h>

vl::ogre::Root::Root( vl::SettingsRefPtr settings )
	: _ogre_root(0), _primary(false), _settings( settings )
{
	_ogre_root = Ogre::Root::getSingletonPtr();
	if( !_ogre_root )
	{
		std::string plugins = settings->getOgrePluginsPath().file_string();
		_ogre_root = new Ogre::Root( plugins, "" );
		_primary = true;

		std::string msg( "plugin path = " + plugins );
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
	{
		BOOST_THROW_EXCEPTION( vl::exception() << vl::desc("No OpenGL rendering system plugin found") );
	}
	else
	{ _ogre_root->setRenderSystem( rast ); }
}

void
vl::ogre::Root::init( void )
{
	_ogre_root->initialise( false );
}

/// Method which will define the source of resources (other than current folder)
void
vl::ogre::Root::setupResources( void )
{
	std::string msg( "setupResources" );
	Ogre::LogManager::getSingleton().logMessage( msg );

	std::vector<fs::path> resources = _settings->getOgreResourcePaths();
	for( size_t i = 0; i < resources.size(); ++i )
	{
		setupResource( resources.at(i).file_string() );
	}
}

void
vl::ogre::Root::loadResources(void)
{
	// Initialise, parse scripts etc
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}

void
vl::ogre::Root::setupResource( fs::path const &file )
{
	std::string msg( "Using resource file = ");
	msg += file.file_string();
	Ogre::LogManager::getSingleton().logMessage( msg );

	// Load resource paths from config file
	Ogre::ConfigFile cf;
	cf.load( file.file_string() );

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
					file.parent_path().file_string() + "/" + archName,
					typeName, secName);
#endif
		}
	}
}

Ogre::RenderWindow *
vl::ogre::Root::createWindow( std::string const &name, unsigned int width,
		unsigned int height, vl::NamedValuePairList const &params )
{
	if( !_ogre_root )
	{ 
		//return vl::graph::RenderWindowRefPtr(); 
		return 0;
	}

	static int n_windows = 0;
			
	Ogre::NameValuePairList misc;
	vl::NamedValuePairList::const_iterator iter = params.begin();
	for( ; iter != params.end(); ++iter )
	{ misc[iter->first] = iter->second; }

	std::stringstream ss( std::stringstream::in | std::stringstream::out );
	ss << name << "-" << n_windows;
	Ogre::RenderWindow *og_win =
		_ogre_root->createRenderWindow( ss.str(), width, height, false, &misc );
	++n_windows;

	// Initialise the rendering system and load resources automatically when
	// first window is created.
	if( !_ogre_root->isInitialised() )
	{ init(); }

//	vl::graph::RenderWindowRefPtr win( new vl::ogre::RenderWindow( og_win ) );
	return og_win;
}

Ogre::SceneManager *
vl::ogre::Root::createSceneManager(std::string const &name )
{
	EQASSERT( _ogre_root );

	Ogre::SceneManager *og_man 
		= _ogre_root->createSceneManager( Ogre::ST_GENERIC, name );
//	EQASSERT( og_man );
//	vl::graph::SceneManagerRefPtr man( 
//		new vl::ogre::SceneManager( og_man, name ) );
	return og_man;
}
