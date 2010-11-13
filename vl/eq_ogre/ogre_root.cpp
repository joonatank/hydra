
// Interface include
#include "ogre_root.hpp"

// library includes
#include "base/exceptions.hpp"

// Ogre includes
#include <OGRE/OgreResourceManager.h>
#include <OGRE/OgreLogManager.h>

#include "base/helpers.hpp"

vl::ogre::Root::Root( vl::SettingsRefPtr settings )
	: _ogre_root(0), _log_manager(0), _primary(false), _settings( settings )
{
	_ogre_root = Ogre::Root::getSingletonPtr();
	if( !_ogre_root )
	{
		_log_manager = new Ogre::LogManager();

		// Get the log file path
		std::stringstream log_file_name_stream;
		if( !settings->getLogDir().empty() )
		{
			log_file_name_stream << settings->getLogDir() << "/";
		}
		// TODO add project name to log file
		log_file_name_stream << "project_name" << "_ogre_" << vl::getPid() << ".log";

		// Create the log
		Ogre::Log *log = Ogre::LogManager::getSingleton().createLog( log_file_name_stream.str(), true, false );
		log->setTimeStampEnabled( true );
		log->logMessage( std::string("Log file path = ") + log_file_name_stream.str() );

		_ogre_root = new Ogre::Root( "", "", "" );
		_primary = true;
	}
}

vl::ogre::Root::~Root( void )
{
	// FIXME this can not destroy ogre root if we have multiple
	// Roots pointing to same ogre singleton.
	if( _primary )
	{ delete _ogre_root; }
	delete _log_manager;
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

	//std::vector<std::string> resources = _settings->getOgreResourcePaths();
	std::stringstream ss;
	fs::path proj_file = fs::path( _settings->getProjectSettings()->getFile() );
	ss << "project file = " << proj_file.file_string();
	Ogre::LogManager::getSingleton().logMessage( ss.str() );
	ss.str("");

	fs::path resource_dir = proj_file.parent_path() / "resources";
	ss << "resource dir = " << resource_dir.file_string();
	Ogre::LogManager::getSingleton().logMessage( ss.str() );
	ss.str("");

	// Throw a generic exception here for now
	if( !fs::exists( resource_dir ) )
	{
		BOOST_THROW_EXCEPTION( vl::missing_dir() << vl::file_name( resource_dir.file_string() ) );
	}

	// Add the root resource dir
	_setupResource( resource_dir.file_string(), "FileSystem" );
	// Add all child resources
	_iterateResourceDir( resource_dir );
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
	// FIXME should find the RenderSystem_GL plugin, both in Windows and Linux
	// Both : {Environment file dir}/plugins, current dir
	// Windows : ${PATH}, ${PATH}/OGRE
	// Linux : /usr/lib, /usr/local/lib, /usr/lib/OGRE, /usr/local/lib/OGRE
	_ogre_root->loadPlugin( "/usr/local/lib/OGRE/RenderSystem_GL.so" );
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
		}
	}
}

void
vl::ogre::Root::_setupResource( std::string const &file, std::string const &typeName)
{
	std::string msg = "Adding resource = " + file
		+ " of type "+ typeName;
	Ogre::LogManager::getSingleton().logMessage( msg );

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
	// OS X does not set the working directory relative to the app,
	// In order to make things portable on OS X we need to provide
	// the loading with it's own bundle path location
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
		std::string(macBundlePath() + "/" + file), typeName );
#else
	Ogre::ResourceGroupManager::getSingleton()
		.addResourceLocation( file, typeName );
#endif
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
