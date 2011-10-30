/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
 *
 */

// Interface include
#include "ogre_root.hpp"

// library includes
#include "base/exceptions.hpp"

// Ogre includes
#include <OGRE/OgreResourceManager.h>
#include <OGRE/OgreLogManager.h>

#include "base/system_util.hpp"
#include "base/filesystem.hpp"

/// Necessary for logging levels
#include "logger.hpp"

vl::ogre::Root::Root(vl::config::LogLevel level)
	: _ogre_root(0),
	  _log_manager(0),
	  _primary(false)
{
	std::cout << vl::TRACE << "vl::ogre::Root::Root" << std::endl;

	_ogre_root = Ogre::Root::getSingletonPtr();
	if( !_ogre_root )
	{
		// TODO move the logger to Pipe
		_log_manager = new Ogre::LogManager();

		// Create the log
		Ogre::Log *log = Ogre::LogManager::getSingleton()
			.createLog("", true, true, true);

		Ogre::LoggingLevel ol;
		if( level == vl::config::LL_LOW ) ol = Ogre::LL_LOW;
		else if( level == vl::config::LL_NORMAL ) ol = Ogre::LL_NORMAL;
		else if( level == vl::config::LL_BOREME ) ol = Ogre::LL_BOREME;

		Ogre::LogManager::getSingleton().setLogDetail(ol);

		std::cout << vl::TRACE << "vl::ogre::Root::Root : create Ogre Root" << std::endl;
		_ogre_root = new Ogre::Root( "", "", "" );
		_primary = true;
	}

	std::cout << vl::TRACE << "vl::ogre::Root::Root : done" << std::endl;
}

vl::ogre::Root::~Root( void )
{
	// root and log manager point to same ogre singletons.
	// destroy them only on the primary
	if( _primary )
	{
		delete _ogre_root;
		delete _log_manager;
	}
}

void
vl::ogre::Root::createRenderSystem( void )
{
	std::cout << vl::TRACE << "vl::ogre::Root::createRenderSystem" << std::endl;

	if( !_primary )
	{ return; }

	if( !_ogre_root )
	{ BOOST_THROW_EXCEPTION( vl::exception() ); }

	_loadPlugins();


	// We only support OpenGL rasterizer
	Ogre::RenderSystem *rast
		= _ogre_root->getRenderSystemByName( "OpenGL Rendering Subsystem" );
	if( !rast )
	{
		std::string err_desc( "No OpenGL rendering system plugin found" );
		BOOST_THROW_EXCEPTION( vl::exception() << vl::desc(err_desc) );
	}
	else
	{ _ogre_root->setRenderSystem( rast ); }
}

void
vl::ogre::Root::init( void )
{
	_ogre_root->initialise( false );
}

void
vl::ogre::Root::addResource(const std::string& resource_path)
{
	_resources.push_back( resource_path );

	// TODO this is confusing as this does not really setup the resource
	// but adds it into stack which is not cleared at any point
	// so calling first this one, then setupResources, then this again and
	// again setupResources will cause the first entry to be added twice to the
	// resources.
}

/// Method which will define the source of resources (other than current folder)
void
vl::ogre::Root::setupResources( void )
{
	std::string msg( "vl::ogre::Root::setupResources" );
	Ogre::LogManager::getSingleton().logMessage( msg, Ogre::LML_TRIVIAL );

	for( std::vector<std::string>::iterator iter = _resources.begin();
		iter != _resources.end(); ++iter )
	{
		// This should never happen as the resource paths settings provides
		// should be valid.
		if( !fs::is_directory( *iter ) )
		{
			BOOST_THROW_EXCEPTION( vl::missing_dir() << vl::file_name( *iter ) );
		}

		// Add all the child directories and zip archives
		_setupResourceDir(*iter);
	}
}

void
vl::ogre::Root::loadResources(void)
{
	// Initialise, parse scripts etc
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}

/// Private

void
vl::ogre::Root::_loadPlugins(void )
{
	std::string msg( "_loadPlugins" );
	Ogre::LogManager::getSingleton().logMessage( msg, Ogre::LML_TRIVIAL );

	// TODO add support for plugins in the EnvSettings


// Check if this is a debug version, only Windows uses debug versions of the libraries
// So we need to load the debug versions of the Ogre plugins only on Windows.
#if defined(_WIN32) && defined(_DEBUG)
	std::string gl_plugin_name( "RenderSystem_GL_d" );
#else
	std::string gl_plugin_name( "RenderSystem_GL" );
#endif

	std::string plugin_path = vl::findPlugin( gl_plugin_name );
	/// @todo this should throw if not found, because we don't have a rendering system
	if( !plugin_path.empty() )
	{ _ogre_root->loadPlugin( plugin_path ); }
}

void
vl::ogre::Root::_setupResourceDir( const std::string& dir )
{
	fs::directory_iterator endIter;
	for( fs::directory_iterator dirIter( dir ); dirIter != endIter; ++dirIter )
	{
		if( fs::is_directory( dirIter->path() ) )
		{
			_setupResourceDir(dirIter->path().string() );
		}
		else if( dirIter->path().extension() == ".zip" )
		{
			_setupResource( dirIter->path().string(), "Zip" );
		}
	}
	// Add the root resource dir
	_setupResource( dir, "FileSystem" );
}


void
vl::ogre::Root::_setupResource( std::string const &file, std::string const &typeName)
{
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation( file, typeName );
}

Ogre::RenderWindow *
vl::ogre::Root::createWindow( std::string const &name, unsigned int width,
		unsigned int height, Ogre::NameValuePairList const &params )
{
	std::string str("vl::ogre::Root::creatingWindow");
	Ogre::LogManager::getSingleton().logMessage(str, Ogre::LML_TRIVIAL);

	if( !_ogre_root )
	{ return 0; }

	Ogre::RenderWindow *og_win =
		_ogre_root->createRenderWindow( name, width, height, false, &params );

	// Initialise the rendering system and load resources automatically when
	// first window is created.
	if( !_ogre_root->isInitialised() )
	{ init(); }

	return og_win;
}

Ogre::SceneManager *
vl::ogre::Root::createSceneManager(std::string const &name )
{
	if( !_ogre_root )
	{ BOOST_THROW_EXCEPTION( vl::exception() ); }

	Ogre::SceneManager *og_man
		= _ogre_root->createSceneManager( Ogre::ST_GENERIC, name );

	return og_man;
}
