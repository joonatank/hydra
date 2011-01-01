/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2011-01
 *
 *
 */

// Interface
#include "pipe.hpp"

// Necessary for mapping distributed objects
#include <eq/client/config.h>

// Necessary for printing error messages from exceptions
#include "base/exceptions.hpp"
// Necessary for loading dotscene
#include "eq_ogre/ogre_dotscene_loader.hpp"
#include "window.hpp"

/// ------------------------- Public -------------------------------------------
eqOgre::Pipe::Pipe( eq::Node* parent )
	: eq::Pipe(parent), _ogre_sm(0), _camera(0), _screenshot_num(0)
{}

eqOgre::Pipe::~Pipe( void )
{}

eqOgre::DistributedSettings const &
eqOgre::Pipe::getSettings( void ) const
{
	return _settings;
}


/// ------------------------ Protected -----------------------------------------
bool
eqOgre::Pipe::configInit(const eq::uint128_t& initID)
{
	if( !eq::Pipe::configInit(initID) )
	{
		EQERROR << "eq::Pipe::configInit failed" << std::endl;
		return false;
	}

	if( !_mapData( initID ) )
	{
		EQERROR << "eqOgre::Pipe::configInit : mapData failed" << std::endl;
		return false;
	}


	if( !_createOgre() )
	{
		EQERROR << "eqOgre::Pipe::configInit : createOgre failed" << std::endl;
		return false;
	}

	return true;
}

bool
eqOgre::Pipe::configExit()
{
	bool retval = eq::Pipe::configExit();
	_unmapData();

	EQINFO << "Cleaning out OGRE" << std::endl;
	_root.reset();

	return retval;
}

void
eqOgre::Pipe::frameStart(const eq::uint128_t& frameID, const uint32_t frameNumber)
{
	// Init the Ogre resources and load a Scene
	// These are here because the RenderWindow needs to be created before loading
	// Meshes
	static bool inited = false;
	if( !inited )
	{
		// Resource registration
		std::vector<std::string> const &resources = _resource_manager.getResourcePaths();
		for( size_t i = 0; i < resources.size(); ++i )
		{
			_root->addResource( resources.at(i) );
		}
		_root->setupResources( );
		_root->loadResources();

		_ogre_sm = _root->createSceneManager("SceneManager");

		if( !_loadScene() )
		{
			EQERROR << "Problem loading the Scene" << std::endl;
		}

		// We need to find the node from scene graph
		EQINFO << "SceneManager has " << _scene_manager.getNSceneNodes()
			<< " SceneNodes." << std::endl;

		EQASSERTINFO( _ogre_sm, "Window has no Ogre SceneManager" );
		if( !_scene_manager.setSceneManager( _ogre_sm ) )
		{
			EQERROR << "Some SceneNodes were not found." << std::endl;
		}

		inited = true;
	}

	_updateDistribData();

	eq::Pipe::frameStart( frameID, frameNumber );
}


/// Ogre helpers
bool
eqOgre::Pipe::_createOgre( void )
{
	try {
		EQINFO << "Creating Ogre Root" << std::endl;

		_root.reset( new vl::ogre::Root( getSettings().getOgreLogFilePath() ) );
		// Initialise ogre
		_root->createRenderSystem();
	}
	catch( vl::exception &e )
	{
		EQERROR << "VL Exception : "<<   boost::diagnostic_information<>(e)
			<< std::endl;
		return false;
	}
	catch( Ogre::Exception const &e)
	{
		EQERROR << "Ogre Exception: " << e.what() << std::endl;
		return false;
	}
	catch( std::exception const &e )
	{
		EQERROR << "STD Exception: " << e.what() << std::endl;
		return false;
	}
	catch( ... )
	{
		std::string err_msg( "eqOgre::Window::configInit : Exception thrown." );
		EQERROR << err_msg << std::endl;
		return false;
	}

	return true;
}

bool
eqOgre::Pipe::_loadScene( void )
{
	// TODO this should be divided to case load scenes function and loadScene function

	// Get scenes
	std::vector<vl::TextResource> scenes = _resource_manager.getSceneResources();
	EQINFO << "Loading Scenes for Project : " << getSettings().getProjectName()
		<< std::endl;

	// If we don't have Scenes there is no point loading them
	if( scenes.empty() )
	{
		EQINFO << "Project does not have any scene files." << std::endl;
		return false;
	}
	else
	{
		EQINFO << "Project has " << scenes.size() << " scene files." << std::endl;
	}

	// Clean up old scenes
	// TODO this should be a loader not a destroyer, move to another function
	_ogre_sm->clearScene();
	_ogre_sm->destroyAllCameras();

	// TODO support for multiple scene files should be tested
	// TODO support for case needs to be tested
	for( size_t i = 0; i < scenes.size(); ++i )
	{
		std::string const &name = scenes.at(i).getName();

		EQINFO << "Loading scene " << name << "." << std::endl;

		eqOgre::DotSceneLoader loader;
		// TODO pass attach node based on the scene
		// TODO add a prefix to the SceneNode names ${scene_name}/${node_name}
		loader.parseDotScene( scenes.at(i), _ogre_sm );

		EQINFO << "Scene " << name << " loaded.";
	}

	/// Get the camera
	// TODO move to separate function

	// Loop through all cameras and grab their name and set their debug representation
	Ogre::SceneManager::CameraIterator cameras = _ogre_sm->getCameraIterator();

	// Grab the first available camera, for now
	if( cameras.hasMoreElements() )
	{
		_camera = cameras.getNext();
		EQINFO << "Using Camera " <<  _camera->getName()
			<< " found from the scene." << std::endl;
	}
	else
	{
		_camera = _ogre_sm->createCamera("Cam");
		EQINFO << "No camera in the scene. Using created camera "
			<< _camera->getName() << std::endl;
	}
	_active_camera_name = _camera->getName();

	return true;
}


/// Distribution helpers
bool
eqOgre::Pipe::_mapData( const eq::uint128_t& settingsID )
{
	EQINFO << "Mapping data." << std::endl;

	// Get the cluster version of data
	if( !getConfig()->mapObject( &_settings, settingsID ) )
	{
		EQERROR << "Couldn't map the Settings." << std::endl;
		return false;
	}

	if( !getConfig()->mapObject( &_resource_manager, _settings.getResourceManagerID() ) )
	{
		EQERROR << "Couldn't map the ResourceManager." << std::endl;
		return false;
	}

	EQASSERT( _settings.getPlayerID() != eq::base::UUID::ZERO );
	if( !getConfig()->mapObject( &_player, _settings.getPlayerID() ) )
	{
		EQERROR << "Couldn't map the Player." << std::endl;
		return false;
	}

	eq::base::UUID const &frame_id = _settings.getSceneManagerID();
	EQASSERT( frame_id != eq::base::UUID::ZERO );
	if( !_scene_manager.mapData( getConfig(), frame_id ) )
	{
		EQERROR << "Couldn't map the SceneManager." << std::endl;
		return false;
	}

	EQINFO << "Data mapped." << std::endl;

	return true;
}

void
eqOgre::Pipe::_unmapData( void )
{
	EQINFO << "Unmapping Settings." << std::endl;
	getConfig()->unmapObject( &_settings );

	EQINFO << "Unmapping ResourceManager" << std::endl;
	getConfig()->unmapObject( &_resource_manager );

	EQINFO << "Unmapping Player." << std::endl;
	getConfig()->unmapObject( &_player );

	EQINFO << "Unmapping SceneManager." << std::endl;
	_scene_manager.unmapData();

}

void
eqOgre::Pipe::_updateDistribData( void )
{
	// Update player
	_player.sync();
	// Get active camera and change the rendering camera if there is a change
	std::string const &cam_name = _player.getActiveCamera();
	if( !cam_name.empty() && cam_name != _active_camera_name )
	{
		_active_camera_name = cam_name;
		if( _ogre_sm->hasCamera( cam_name ) )
		{
			// Tell the Windows to change cameras
			_camera = _ogre_sm->getCamera( _active_camera_name );
			Windows const &window_list = getWindows();
			for( size_t i = 0; i < window_list.size(); ++i )
			{
				EQASSERT( dynamic_cast<eqOgre::Window *>( window_list.at(i) ) );
				eqOgre::Window *window =
					static_cast<eqOgre::Window *>( window_list.at(i) );
				window->setCamera( _camera );
			}
		}
		else
		{
			EQERROR << "eqOgre::Window : New camera name set, but NO camera found"
				<< std::endl;
		}
	}

	// Take a screenshot
	if( _player.getScreenshotVersion() > _screenshot_num )
	{
		// TODO should write the screenshot to the project directory not
		// to current directory
		// Add the screenshot dir to DistributedSettings
		// TODO the format of the screenshot name should be
		// screenshot_{project_name}-{year}-{month}-{day}-{time}-{window_name}.png
		std::string prefix( "screenshot_" );
		std::string suffix = ".png";//

		// Tell the Windows to take a screenshot
		Windows const &window_list = getWindows();
		for( size_t i = 0; i < window_list.size(); ++i )
		{
			EQASSERT( dynamic_cast<eqOgre::Window *>( window_list.at(i) ) );
			eqOgre::Window *window =
				static_cast<eqOgre::Window *>( window_list.at(i) );
			window->takeScreenshot( prefix, suffix );
		}

		_screenshot_num = _player.getScreenshotVersion();
	}

	// Update SceneManager
	_scene_manager.syncAll();

	// FIXME this is completely screwed up.
	/*
	static uint32_t scene_version = 0;
	if( _frame_data.getSceneVersion() > scene_version )
	{
		// This will reload the scene but all transformations remain
		// As this will not reset the SceneNode structures that control the
		// transformations of objects.
		EQINFO << "Reloading the Ogre scene now" << std::endl;
		eqOgre::Window *win = static_cast<eqOgre::Window *>( getWindow() );
		win->loadScene();
		Ogre::Camera *camera = win->getCamera();
		createViewport( camera );
		_frame_data.setSceneManager( win->getSceneManager() );
		EQINFO << "Ogre Scene reloaded." << std::endl;

		scene_version = _frame_data.getSceneVersion();
	}
	*/

}
