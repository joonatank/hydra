/**
 *	Copyright (c) 2010 - 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-11
 *	@file: ogre_root.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
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
	: _ogre_root(0)
	, _log_manager(0)
	, _scene_manager(0)
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
	}

	std::cout << vl::TRACE << "vl::ogre::Root::Root : done" << std::endl;
}

vl::ogre::Root::~Root( void )
{
	_ogre_root->destroySceneManager(_scene_manager);

	delete _ogre_root;
	delete _log_manager;
}

void
vl::ogre::Root::createRenderSystem( void )
{
	std::cout << vl::TRACE << "vl::ogre::Root::createRenderSystem" << std::endl;

	if( !_ogre_root )
	{ BOOST_THROW_EXCEPTION( vl::exception() ); }

	_loadPlugins();


	// We only support OpenGL rasterizer
	
	// Try our own rasterizer first
	std::string rasterizer_name = "Hydra OpenGL Rasterizer";
	Ogre::RenderSystem *rast = _ogre_root->getRenderSystemByName(rasterizer_name);
	if(!rast)
	{
		// Fallback to Ogres
		rasterizer_name = "OpenGL Rendering Subsystem";
		rast = _ogre_root->getRenderSystemByName(rasterizer_name);

		if(!rast)
		{
			std::string err_desc( "No OpenGL rendering system plugin found" );
			BOOST_THROW_EXCEPTION( vl::exception() << vl::desc(err_desc) );
		}
	}
	
	_ogre_root->setRenderSystem( rast );
}

void
vl::ogre::Root::init( void )
{
	_ogre_root->initialise( false );
}

void
vl::ogre::Root::setupResources(std::vector<std::string> const &paths)
{
	std::clog << "vl::ogre::Root::setupResources" << std::endl;

	for( std::vector<std::string>::const_iterator iter = paths.begin();
		iter != paths.end(); ++iter )
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
vl::ogre::Root::removeResources(void)
{
	Ogre::ResourceGroupManager::getSingleton().clearResourceGroup(Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
}

void
vl::ogre::Root::loadResources(void)
{
	// Initialise, parse scripts etc
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
	
}

/// Private

void
vl::ogre::Root::_loadPlugins(void)
{
	std::cout << vl::TRACE << "vl::ogre::Root::_loadPlugins" << std::endl;

#if defined(_WIN32) && defined(_DEBUG)
	std::string gl_plugin_name("HydraGL_d");
#else
	std::string gl_plugin_name("HydraGL");
#endif

	std::string plugin_path = vl::findPlugin( gl_plugin_name );
	
	/// Fallback to Ogre GL plugin
	if(plugin_path.empty())
	{
// Check if this is a debug version, only Windows uses debug versions of the libraries
// So we need to load the debug versions of the Ogre plugins only on Windows.
#if defined(_WIN32) && defined(_DEBUG)
		gl_plugin_name = "RenderSystem_GL_d";
#else
		gl_plugin_name = "RenderSystem_GL";
#endif

		plugin_path = vl::findPlugin( gl_plugin_name );
	}

	/// @todo this should throw if not found, because we don't have a rendering system
	if(plugin_path.empty())
	{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Neither OpenGL plugins found.")); }

	_ogre_root->loadPlugin( plugin_path );
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
	if(fs::path(file).leaf() == "SkyX")
	{
		std::clog << "Setting up SkyX resource." << std::endl;
		Ogre::ResourceGroupManager::getSingleton().addResourceLocation( file, typeName,"SkyX" );
	}
	else
	{
		Ogre::ResourceGroupManager::getSingleton().addResourceLocation( file, typeName );
	}
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
	{ BOOST_THROW_EXCEPTION(vl::null_pointer() << vl::desc("Ogre Root hasn't been created yet.")); }
	if(_scene_manager)
	{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Only single Scene Manager is supported at this point.")); }
	
	_scene_manager = _ogre_root->createSceneManager( Ogre::ST_GENERIC, name );

	return _scene_manager;
}

Ogre::SceneManager *
vl::ogre::Root::getSceneManager(void) const
{
	return _scene_manager;
}
