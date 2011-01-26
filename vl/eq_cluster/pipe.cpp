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
#include "base/string_utils.hpp"

char const *SERVER_NAME = "localhost";
uint16_t const SERVER_PORT = 4699;

/// ------------------------- Public -------------------------------------------
eqOgre::Pipe::Pipe( eq::Node* parent )
	: eq::Pipe(parent), _ogre_sm(0), _camera(0), _screenshot_num(0), _client(0)
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
		// Error
		// TODO move to using Ogre Logging, the log manager needs to be created sooner
		std::string message("eq::Pipe::configInit failed");
		std::cerr << message << std::endl;
		return false;
	}

	if( !_mapData( initID ) )
	{
		// Error
		// TODO move to using Ogre Logging, the log manager needs to be created sooner
		std::string message("eqOgre::Pipe::configInit : mapData failed");
		std::cerr << message << std::endl;
		return false;
	}


	if( !_createOgre() )
	{
		// Error
		// TODO move to using Ogre Logging, the log manager needs to be created sooner
		std::string message("eqOgre::Pipe::configInit : createOgre failed");
		std::cerr << message << std::endl;
		return false;
	}

	_createClient();

	return true;
}

bool
eqOgre::Pipe::configExit()
{
	bool retval = eq::Pipe::configExit();
	_unmapData();

	// Info
	std::string message("Cleaning out OGRE");
	Ogre::LogManager::getSingleton().logMessage( message );
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
			// Error
			std::string message("Problem loading the Scene");
			Ogre::LogManager::getSingleton().logMessage( message, Ogre::LML_CRITICAL );
		}

		// We need to find the node from scene graph
		std::string message = "SceneManager has "
			+ vl::to_string(_scene_manager->getNSceneNodes()) + " SceneNodes.";
		Ogre::LogManager::getSingleton().logMessage( message );

		EQASSERTINFO( _ogre_sm, "Window has no Ogre SceneManager" );
		if( !_scene_manager->setSceneManager( _ogre_sm ) )
		{
			// Error
			message = "Some SceneNodes were not found.";
			Ogre::LogManager::getSingleton().logMessage( message, Ogre::LML_CRITICAL );
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
		// TODO needs LogManager creation before this
//		std::string message("Creating Ogre Root");
//		Ogre::LogManager::getSingleton().logMessage( message );

		_root.reset( new vl::ogre::Root( getSettings().getOgreLogFilePath() ) );
		// Initialise ogre
		_root->createRenderSystem();
	}
	catch( vl::exception &e )
	{
		std::string message = "VL Exception : " + boost::diagnostic_information<>(e);
		Ogre::LogManager::getSingleton().logMessage( message, Ogre::LML_CRITICAL );

		return false;
	}
	catch( Ogre::Exception const &e)
	{
		std::string message = std::string("Ogre Exception: ") + e.what();
		Ogre::LogManager::getSingleton().logMessage( message, Ogre::LML_CRITICAL );

		return false;
	}
	catch( std::exception const &e )
	{
		std::string message = std::string("STD Exception: ") + e.what();
		Ogre::LogManager::getSingleton().logMessage( message, Ogre::LML_CRITICAL );

		return false;
	}
	catch( ... )
	{
		std::string err_msg( "eqOgre::Window::configInit : Exception thrown." );
		Ogre::LogManager::getSingleton().logMessage( err_msg, Ogre::LML_CRITICAL );
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
	std::string message = "Loading Scenes for Project : " + getSettings().getProjectName();
	Ogre::LogManager::getSingleton().logMessage( message );

	// If we don't have Scenes there is no point loading them
	if( scenes.empty() )
	{
		message = "Project does not have any scene files.";
		Ogre::LogManager::getSingleton().logMessage( message, Ogre::LML_CRITICAL );
		return false;
	}
	else
	{
		message = "Project has " + vl::to_string(scenes.size()) + " scene files.";
		Ogre::LogManager::getSingleton().logMessage( message );
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

		message = "Loading scene " + name + ".";
		Ogre::LogManager::getSingleton().logMessage( message );

		eqOgre::DotSceneLoader loader;
		// TODO pass attach node based on the scene
		// TODO add a prefix to the SceneNode names ${scene_name}/${node_name}
		loader.parseDotScene( scenes.at(i), _ogre_sm );

		message = "Scene " + name + " loaded.";
		Ogre::LogManager::getSingleton().logMessage( message );
	}

	/// Get the camera
	// TODO move to separate function

	// Loop through all cameras and grab their name and set their debug representation
	Ogre::SceneManager::CameraIterator cameras = _ogre_sm->getCameraIterator();

	// Grab the first available camera, for now
	if( cameras.hasMoreElements() )
	{
		_camera = cameras.getNext();
		message = "Using Camera " + _camera->getName()
			+ " found from the scene.";
		Ogre::LogManager::getSingleton().logMessage( message );
	}
	else
	{
		_camera = _ogre_sm->createCamera("Cam");
		message = "No camera in the scene. Using created camera "
			+ _camera->getName();
		Ogre::LogManager::getSingleton().logMessage( message );
	}
	_active_camera_name = _camera->getName();

	return true;
}


/// Distribution helpers
void
eqOgre::Pipe::_createClient( void )
{
	std::cout << "eqOgre::Pipe::_createClient" << std::endl;
	EQASSERT( !_client );

	// FIXME these should be configured in config file
	_client = new vl::cluster::Client( SERVER_NAME, SERVER_PORT );
	_client->registerForUpdates();
}

void
eqOgre::Pipe::_syncData( void )
{
// 	std::cout << "eqOgre::Pipe::_syncData" << std::endl;

	EQASSERT( _client );

	_client->mainloop();
	while( _client->messages() )
	{
//		std::cout << "eqOgre::Pipe::_syncData : Messages received" << std::endl;
		vl::cluster::Message *msg = _client->popMessage();
		// TODO process the message
		switch( msg->getType() )
		{
			case vl::cluster::MSG_UPDATE :
				// Read the IDs in the message and call pack on mapped objects
				// based on thoses
				// TODO multiple update messages in the same frame,
				// only the most recent should be used.
				while( msg->size() > 0 )
				{
// 					std::cout << "eqOgre::Pipe::_syncData : UPDATE message : "
// 						<< "size = " << msg->size() << std::endl;
					uint64_t id;
					msg->read(id);
// 					std::cout << "Object ID = " << id << std::endl;
// 					std::cout << "Mapped objects size = "
// 						<< _mapped_objects.size() << std::endl;
					vl::Distributed *obj = findMappedObject(id);
					if( obj )
					{ obj->unpack(*msg); }
					else
					{
						std::cerr << "No ID " << id << " found in mapped objects."
							<< std::endl;
					}
				}
				_scene_manager->finaliseSync();
				break;

			default :
				break;
		}


		delete msg;
	}
}


bool
eqOgre::Pipe::_mapData( const eq::uint128_t& settingsID )
{
	// TODO move to using Ogre Logging, needs LogManager creation
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

	uint64_t sm_id = _settings.getSceneManagerID();
	std::cout << "Mapping SceneManager with id = " << sm_id << '.' << std::endl;
	EQASSERT( sm_id != vl::ID_UNDEFINED );
	_scene_manager = new eqOgre::SceneManager( this );
	mapObjectC( _scene_manager, sm_id );

	EQINFO << "Data mapped." << std::endl;

	return true;
}

void
eqOgre::Pipe::_unmapData( void )
{
	// TODO move to using Ogre Logging, needs LogManager destruction later
	EQINFO << "Unmapping Settings." << std::endl;
	getConfig()->unmapObject( &_settings );

	EQINFO << "Unmapping ResourceManager" << std::endl;
	getConfig()->unmapObject( &_resource_manager );

	EQINFO << "Unmapping Player." << std::endl;
	getConfig()->unmapObject( &_player );

// 	EQINFO << "Unmapping SceneManager." << std::endl;
// 	_scene_manager.unmapData();

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
			std::string message = "eqOgre::Window : New camera name set, but NO camera found";
			Ogre::LogManager::getSingleton().logMessage( message );
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
	// FIXME this should use the custom Client object which should be connected
	// to the Server
	_syncData();
// 	_scene_manager.syncAll();

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
