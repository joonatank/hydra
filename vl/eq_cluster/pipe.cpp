/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-01
 *	@file pipe.cpp
 *
 */

// Interface
#include "pipe.hpp"

// Necessary for printing error messages from exceptions
#include "base/exceptions.hpp"
// Necessary for loading dotscene
#include "eq_ogre/ogre_dotscene_loader.hpp"

#include "window.hpp"
#include "base/string_utils.hpp"
#include "base/sleep.hpp"
#include "distrib_settings.hpp"

#include <OGRE/OgreWindowEventUtilities.h>

/// ------------------------- Public -------------------------------------------
// TODO should probably copy the env settings and not store the reference
vl::Pipe::Pipe( std::string const &name,
					std::string const &server_address,
					uint16_t server_port )
	: _name(name)
	, _ogre_sm(0)
	, _camera(0)
	, _scene_manager(0)
	, _player(0)
	, _screenshot_num(0)
	, _client( new vl::cluster::Client( server_address.c_str(), server_port ) )
	, _running(true)	// TODO should this be true from the start?
{
	std::cout << "vl::Pipe::Pipe : name = " << _name << std::endl;

	_client->registerForUpdates();
}

vl::Pipe::~Pipe( void )
{
	std::cout << "vl::Pipe::~Pipe" << std::endl;

	// Some asserts for checking that the Pipe thread has been correctly shutdown
	assert( !_root );

	delete _scene_manager;
	delete _player;

	_client.reset();

	std::cout << "vl::Pipe::~Pipe : DONE" << std::endl;
}

vl::EnvSettingsRefPtr
vl::Pipe::getSettings( void )
{
	return _env;
}

vl::EnvSettings::Node
vl::Pipe::getNodeConf( void )
{
	vl::EnvSettings::Node node;
	if( getName() == _env->getMaster().name )
	{ node = _env->getMaster(); }
	else
	{ node = _env->findSlave( getName() ); }

	assert( !node.empty() );
	return node;
}

vl::EnvSettings::Window
vl::Pipe::getWindowConf( std::string const &window_name )
{
	vl::EnvSettings::Node node = getNodeConf();

	// TODO add real errors
	assert( node.getNWindows() > 0 );

	for( size_t i = 0; i < node.getNWindows(); ++i )
	{
		if( node.getWindow(i).name == window_name )
		{ return node.getWindow(i); }
	}

	// TODO add real errors
	assert( false );
}

void
vl::Pipe::sendMessageToMaster( vl::cluster::Message *msg )
{
	_client->sendMessage(msg);
}

void
vl::Pipe::sendEvent( vl::cluster::EventData const &event )
{
	// Add to event stack for sending them at once in one message to the Master
	_events.push_back(event);
}

void
vl::Pipe::operator()()
{
	// Here we should wait for the EnvSettings from master
	// TODO we should have a wait for Message function
	while( !_env )
	{
		_handleMessages();
		boost::this_thread::sleep( boost::posix_time::milliseconds(1) );
	}

	_createOgre();

	vl::EnvSettings::Node node = getNodeConf();
	std::string msg = "Creating " + vl::to_string(node.getNWindows()) + " windows.";
	Ogre::LogManager::getSingleton().logMessage( msg, Ogre::LML_TRIVIAL );

	for( size_t i = 0; i < node.getNWindows(); ++i )
	{ _createWindow( node.getWindow(i) ); }

	while( isRunning() )
	{
		// Handle messages
		_handleMessages();
		if( !isRunning() )
		{ break; }

		// Process input events
		for( size_t i = 0; i < _windows.size(); ++i )
		{ _windows.at(i)->capture(); }

		// Send messages
		_sendEvents();

		// Sleep
		boost::this_thread::sleep( boost::posix_time::milliseconds(1) );
	}
}


/// ------------------------ Protected -----------------------------------------
void
vl::Pipe::_reloadProjects( vl::Settings set )
{
	// TODO this should unload old projects

	_settings = set;

	std::vector<vl::ProjSettings::Scene> scenes = _settings.getScenes();

	// Add resources
	_root->addResource( _settings.getProjectDir() );
	for( size_t i = 0; i < _settings.getAuxDirectories().size(); ++i )
	{
		_root->addResource( _settings.getAuxDirectories().at(i) );
	}

	std::string msg("Setting up the resources.");
	Ogre::LogManager::getSingleton().logMessage( msg, Ogre::LML_TRIVIAL );

	_root->setupResources();
	_root->loadResources();

	_ogre_sm = _root->createSceneManager("SceneManager");

	for( size_t i = 0; i < scenes.size(); ++i )
	{
		_loadScene( scenes.at(i) );
	}

	_setCamera();
}

/// Ogre helpers
void
vl::Pipe::_createOgre( void )
{
	assert( _env );

	// TODO the project name should be used instead of the hydra for all
	// problem is that the project name is not known at this point
	// so we should use a tmp file and then move it.
	std::string log_file = vl::createLogFilePath( "hydra", "ogre", "", _env->getLogDir() );
	_root.reset( new vl::ogre::Root( log_file, _env->getVerbose() ) );
	// Initialise ogre
	_root->createRenderSystem();
}

void
vl::Pipe::_loadScene( vl::ProjSettings::Scene const &scene )
{
	std::string msg("vl::Pipe::_loadScene");
	Ogre::LogManager::getSingleton().logMessage( msg, Ogre::LML_TRIVIAL );

	assert( _ogre_sm );

	std::string const &name = scene.getName();

	msg = "Loading scene " + name + " file = " + scene.getFile();
 	Ogre::LogManager::getSingleton().logMessage( msg, Ogre::LML_NORMAL );

	vl::ogre::DotSceneLoader loader;
	// TODO pass attach node based on the scene
	// TODO add a prefix to the SceneNode names ${scene_name}/${node_name}
	loader.parseDotScene( scene.getFile(), _ogre_sm );

	msg = "Scene " + name + " loaded.";
	Ogre::LogManager::getSingleton().logMessage( msg, Ogre::LML_NORMAL );
}

void
vl::Pipe::_setCamera ( void )
{
	assert( _ogre_sm );

	/// Get the camera
	// TODO move to separate function

	// Loop through all cameras and grab their name and set their debug representation
	Ogre::SceneManager::CameraIterator cameras = _ogre_sm->getCameraIterator();

	if( !_active_camera_name.empty() )
	{
		if( _camera && _camera->getName() == _active_camera_name )
		{ return; }

		if( _ogre_sm->hasCamera(_active_camera_name) )
		{
			_camera = _ogre_sm->getCamera(_active_camera_name);
		}
	}

	if( !_camera )
	{
		std::string message;
		// Grab the first available camera, for now
		if( cameras.hasMoreElements() )
		{
			_camera = cameras.getNext();
			message = "Using Camera " + _camera->getName() + " found from the scene.";
			Ogre::LogManager::getSingleton().logMessage( message );
		}
		else
		{
			// TODO this should use a default camera created earlier that exists always
			_camera = _ogre_sm->createCamera("Cam");
			message = "No camera in the scene. Using created camera "
				+ _camera->getName();
			Ogre::LogManager::getSingleton().logMessage( message );
		}
		_active_camera_name = _camera->getName();
	}

	assert( _camera && _active_camera_name == _camera->getName() );
	for( size_t i = 0; i < _windows.size(); ++i )
	{ _windows.at(i)->setCamera(_camera); }

	std::string msg = "Camera " + _active_camera_name + " set.";
	Ogre::LogManager::getSingleton().logMessage( msg );
}


/// Distribution helpers
void
vl::Pipe::_handleMessages( void )
{
// 	std::cout << "vl::Pipe::_handleMessages" << std::endl;

	assert( _client );

	_client->mainloop();
	while( _client->messages() )
	{
// 		std::cout << "vl::Pipe::_handleMessages : Messages received" << std::endl;
		vl::cluster::Message *msg = _client->popMessage();
		// TODO process the message
		_handleMessage(msg);
		delete msg;
	}
}

void
vl::Pipe::_handleMessage( vl::cluster::Message *msg )
{
	assert(msg);

	switch( msg->getType() )
	{
		// Environment configuration
		case vl::cluster::MSG_ENVIRONMENT :
		{
			std::cout << "vl::Pipe::_handleMessage : MSG_ENVIRONMENT message" << std::endl;
			assert( !_env );
			_env.reset( new vl::EnvSettings );
			// TODO needs a ByteData object for Environment settings
			vl::SettingsByteData data;
			data.copyFromMessage(msg);
 			vl::cluster::ByteStream stream(&data);
			stream >> _env;
			// Only single environment settings should be in the message
			assert( 0 == msg->size() );
			_client->sendAck( vl::cluster::MSG_ENVIRONMENT );
		}
		break;

		// Project configuration
		case vl::cluster::MSG_PROJECT :
		{
			std::cout << "vl::Pipe::_handleMessage : MSG_PROJECT message" << std::endl;
			// TODO
			// Test using multiple projects e.g. project and global
			// The vector serailization might not work correctly
			// TODO
			// Problematic because the Project config should be
			// updatable during the application run
			// And this one will create them anew, so that we need to invalidate
			// the scene and reload everything
			// NOTE
			// Combining the project configurations is not done automatically
			// so they either need special structure or we need to iterate over
			// all of them always.
			// TODO needs a ByteData object for Environment settings
			vl::SettingsByteData data;
			data.copyFromMessage(msg);
			vl::cluster::ByteStream stream(&data);
			vl::Settings projects;
			stream >> projects;
			_reloadProjects(projects);
			// TODO should the ACK be first so that the server has the
			// information fast
			_client->sendAck( vl::cluster::MSG_PROJECT );
		}
		break;

		case vl::cluster::MSG_SG_CREATE :
		{
			_client->sendAck( vl::cluster::MSG_SG_CREATE );
			_handleCreateMsg(msg);
		}
		break;
		
		// Scene graph update after the initial message
		case vl::cluster::MSG_SG_UPDATE :
		{
			_client->sendAck( vl::cluster::MSG_SG_UPDATE );
			_handleUpdateMsg(msg);
			_syncData();
			_updateDistribData();
		}
		break;

		case vl::cluster::MSG_DRAW :
		{
			_client->sendAck( vl::cluster::MSG_DRAW );
			_draw();
		}
		break;

		case vl::cluster::MSG_SWAP :
		{
			_client->sendAck( vl::cluster::MSG_SWAP );
			_swap();
		}
		break;

		case vl::cluster::MSG_SHUTDOWN :
		{
			std::cout << "MSG_SHUTDOWN received" << std::endl;
			_client->sendAck( vl::cluster::MSG_SHUTDOWN );
			_shutdown();
		}
		break;

		default :
			std::cout << "Unhandled Message of type = " << msg->getType()
				<< std::endl;
			break;
	}
}

void 
vl::Pipe::_handleCreateMsg( vl::cluster::Message *msg )
{
	std::cout << "vl::Pipe::_handleCreateMsg" << std::endl;
	size_t size;
	assert( msg->size() >= sizeof(size) );

	msg->read( size );
	for( size_t i = 0; i < size; ++i )
	{
		assert( msg->size() > 0 );
		OBJ_TYPE type;
		uint64_t id;

		msg->read(type);
		msg->read(id);
		
		if( OBJ_PLAYER == type )
		{
			// TODO support multiple players
			assert( !_player );
			// TODO fix the constructor
			_player = new vl::Player;
			mapObject( _player, id );
		}
		else if( OBJ_SCENE_MANAGER == type )
		{
			// TODO support multiple SceneManagers
			assert( !_scene_manager );
			_scene_manager = new SceneManager( this, id );
			assert( _ogre_sm );
			_scene_manager->setSceneManager( _ogre_sm );
		}
		else if( OBJ_SCENE_NODE == type )
		{
			assert( _scene_manager );
			_scene_manager->createSceneNode( "", id );
		}
	}
}

void
vl::Pipe::_handleUpdateMsg( vl::cluster::Message* msg )
{
	// Read the IDs in the message and call pack on mapped objects
	// based on thoses
	// @TODO multiple update messages in the same frame,
	// only the most recent should be used.
	while( msg->size() > 0 )
	{
		vl::cluster::ObjectData data;
		data.copyFromMessage(msg);
		// Pushing back will create copies which is unnecessary
		_objects.push_back(data);
	}
}

void
vl::Pipe::_syncData( void )
{
	// TODO remove the temporary array
	// use a custom structure that does not create temporaries
	// rather two phase system one to read the array and mark objects for delete
	// and second that really clear those that are marked for delete
	// similar system for reading data to the array

	// Temporary array used for objects not yet found and saved for later use
	std::vector<vl::cluster::ObjectData> tmp;
	std::vector<vl::cluster::ObjectData>::iterator iter;
	for( iter = _objects.begin(); iter != _objects.end(); ++iter )
	{
		vl::cluster::ByteStream stream = iter->getStream();
		vl::Distributed *obj = findMappedObject( iter->getId() );
		if( obj )
		{
			obj->unpack(stream);
		}
		else
		{
			std::cerr << "No ID " << iter->getId() << " found in mapped objects."
				<< std::endl;
			tmp.push_back( *iter );
		}
	}

	_objects = tmp;
}

void
vl::Pipe::_updateDistribData( void )
{
// 	std::cout << "vl::Pipe::_updateDistribData" << std::endl;

	if( !_player )
	{ return; }

	// Update player
	// Get active camera and change the rendering camera if there is a change
	std::string const &cam_name = _player->getActiveCamera();
	if( !cam_name.empty() && cam_name != _active_camera_name )
	{
		_active_camera_name = cam_name;
		assert( _ogre_sm );
		if( _ogre_sm->hasCamera( cam_name ) )
		{
			// Tell the Windows to change cameras
			_camera = _ogre_sm->getCamera( _active_camera_name );
			assert( !_windows.empty() );
			for( size_t i = 0; i < _windows.size(); ++i )
			{ _windows.at(i)->setCamera( _camera ); }
		}
		else
		{
			std::string message = "vl::Window : New camera name set, but NO camera found";
			std::cout << message << std::endl;
			Ogre::LogManager::getSingleton().logMessage( message );
		}
	}

	// Take a screenshot
	if( _player->getScreenshotVersion() > _screenshot_num )
	{
		_takeScreenshot();

		_screenshot_num = _player->getScreenshotVersion();
	}
}

void
vl::Pipe::_draw( void )
{
	Ogre::WindowEventUtilities::messagePump();
	for( size_t i = 0; i < _windows.size(); ++i )
	{ _windows.at(i)->draw(); }
}

void
vl::Pipe::_swap( void )
{
	for( size_t i = 0; i < _windows.size(); ++i )
	{ _windows.at(i)->swap(); }
}

void
vl::Pipe::_sendEvents( void )
{
	if( !_events.empty() )
	{
		vl::cluster::Message msg( vl::cluster::MSG_INPUT );
		std::vector<vl::cluster::EventData>::iterator iter;
		for( iter = _events.begin(); iter != _events.end(); ++iter )
		{
			iter->copyToMessage(&msg);
		}
		_events.clear();

		sendMessageToMaster(&msg);
	}
}

void
vl::Pipe::_createWindow( vl::EnvSettings::Window const &winConf )
{
	std::cout << "vl::Pipe::_createWindow : " << winConf.name << std::endl;

	vl::Window *window = new vl::Window( winConf.name, this );
	assert( window );
	_windows.push_back(window);
}

void 
vl::Pipe::_shutdown( void )
{
	std::cout << "vl::Pipe::_shutdown" << std::endl;

	std::vector<Window *>::iterator iter;
	for( iter = _windows.begin(); iter != _windows.end(); ++iter )
	{ delete *iter; }
	std::cout << "vl::Pipe::~Pipe : windows deleted" << std::endl;

	_root->getNative()->destroySceneManager( _ogre_sm );

	// Info
	std::string message("Cleaning out OGRE");
	Ogre::LogManager::getSingleton().logMessage( message );
	_root.reset();

	_running = false;

	std::cout << "vl::Pipe::_shutdown : DONE" << std::endl;
}

void 
vl::Pipe::_takeScreenshot( void )
{
	std::string prefix( "screenshot_" );
	std::string suffix = ".png";

	// Tell the Window to take a screenshot
	for( size_t i = 0; i < _windows.size(); ++i )
	{ _windows.at(i)->takeScreenshot( prefix, suffix ); }
}

void 
vl::PipeThread::operator()()
{
	std::cout << "PipeThread Thread entered." << std::endl;

	assert( !_name.empty() );
	if( _server_address.empty() )
	{ _server_address = "localhost"; }
	assert( _server_port != 0 );

	vl::Pipe *pipe = 0;
	try {
		pipe = new vl::Pipe( _name, _server_address, _server_port );
		pipe->operator()();
	}
	catch( vl::exception &e )
	{
		std::cerr << "VL Exception : "<<   boost::diagnostic_information<>(e)
			<< std::endl;
	}
	catch( Ogre::Exception const &e)
	{
		std::cerr << "Ogre Exception: " << e.what() << std::endl;
	}
	catch( std::exception const &e )
	{
		std::cerr << "STD Exception: " << e.what() << std::endl;
	}
	catch( ... )
	{
		std::cerr << "An exception of unknow type occured." << std::endl;
	}
	
	std::cout << "PipeThread Exited" << std::endl;
	delete pipe;
}
