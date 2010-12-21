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

vl::ogre::Root::Root( eqOgre::DistributedSettings const &settings )
	: _ogre_root(0),
	  _log_manager(0),
	  _primary(false)
{
	_ogre_root = Ogre::Root::getSingletonPtr();
	if( !_ogre_root )
	{
		_log_manager = new Ogre::LogManager();

		// Create the log
		std::string log_file = settings.getOgreLogFilePath();
		Ogre::Log *log = Ogre::LogManager::getSingleton()
			.createLog( log_file, true, false );
		log->setTimeStampEnabled( true );
		std::cout << "Ogre log file path = " << log_file << std::endl;

		_ogre_root = new Ogre::Root( "", "", "" );
		_primary = true;

		// Copy resource paths
		_resources = settings.getResources();
	}
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
	std::string str( "vl::ogre::Root::createRenderSystem" );
	Ogre::LogManager::getSingleton().logMessage( str );

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

/// Method which will define the source of resources (other than current folder)
void
vl::ogre::Root::setupResources( void )
{
	std::string msg( "setupResources" );
	Ogre::LogManager::getSingleton().logMessage( msg );

	if( _resources.empty() )
	{
		BOOST_THROW_EXCEPTION( vl::exception() << vl::desc( "No Resource Paths.") );
	}

	for( std::vector<std::string>::iterator iter = _resources.begin();
		iter != _resources.end(); ++iter )
	{
		std::stringstream ss;
		ss << "Adding resource dir : " << *iter;
		Ogre::LogManager::getSingleton().logMessage( ss.str() );

		// This should never happen as the resource paths settings provides
		// should be valid.
		if( !fs::exists( *iter ) )
		{
			BOOST_THROW_EXCEPTION( vl::missing_dir() << vl::file_name( *iter ) );
		}

		// Add the root resource dir
		_setupResource( *iter, "FileSystem" );
		// Add all child resources
		_iterateResourceDir( *iter );
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
	Ogre::LogManager::getSingleton().logMessage( msg );

	// TODO add support for plugins in the EnvSettings

	
// Check if this is a debug version, only Windows uses debug versions of the libraries
// So we need to load the debug versions of the Ogre plugins only on Windows.
#if defined(_WIN32) && defined(_DEBUG)
	std::string gl_plugin_name( "RenderSystem_GL_d" );
#else
	std::string gl_plugin_name( "RenderSystem_GL" );
#endif

	std::string plugin_path = vl::findPlugin( gl_plugin_name );
	if( !plugin_path.empty() )
		_ogre_root->loadPlugin( plugin_path );
}

void
vl::ogre::Root::_iterateResourceDir( fs::path const &file )
{
	// Iterate the resource directory
	fs::directory_iterator end_itr; // default construction yields past-the-end
	for ( fs::directory_iterator itr( file ); itr != end_itr; ++itr )
	{
		std::string ext = itr->path().extension();
		std::transform( ext.begin(), ext.end(), ext.begin(), ::tolower );
		if ( fs::is_directory(itr->status()) )
		{
			// TODO this needs a recursion

			_setupResource( itr->path().file_string(), "FileSystem" );
			_iterateResourceDir( itr->path() );
		}
		else if( ext == ".zip" )
		{
			_setupResource( itr->path().file_string(), "Zip" );
		}
		else
		{
			// Purposefully empty
		}
	}
}

void
vl::ogre::Root::_setupResource( std::string const &file, std::string const &typeName)
{
	std::string msg = "Adding resource = " + file
		+ " of type "+ typeName;
	Ogre::LogManager::getSingleton().logMessage( msg );

	Ogre::ResourceGroupManager::getSingleton()
		.addResourceLocation( file, typeName );
}

Ogre::RenderWindow *
vl::ogre::Root::createWindow( std::string const &name, unsigned int width,
		unsigned int height, Ogre::NameValuePairList const &params )
{
	std::string str( "vl::ogre::Root::creatingWindow" );
	Ogre::LogManager::getSingleton().logMessage( str );

	if( !_ogre_root )
	{ return 0; }

	static int n_windows = 0;

	std::stringstream ss( std::stringstream::in | std::stringstream::out );
	ss << name << "-" << n_windows;
	Ogre::RenderWindow *og_win =
		_ogre_root->createRenderWindow( ss.str(), width, height, false, &params );
	++n_windows;

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
	{
		BOOST_THROW_EXCEPTION( vl::exception() );
	}

	Ogre::SceneManager *og_man
		= _ogre_root->createSceneManager( Ogre::ST_GENERIC, name );

	return og_man;
}
