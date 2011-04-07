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

#include "gui/window.hpp"
#include "logger.hpp"

#include <OGRE/OgreWindowEventUtilities.h>

/// GUI
#include <CEGUI/CEGUI.h>
#include <CEGUI/RendererModules/Ogre/CEGUIOgreRenderer.h>
#include <CEGUI/CEGUISystem.h>
#include <CEGUI/CEGUIWindow.h>
#include <CEGUI/CEGUIWindowManager.h>
#include <CEGUI/CEGUIDefaultResourceProvider.h>
#include <CEGUI/CEGUIImageset.h>
#include <CEGUI/CEGUIScheme.h>
#include <CEGUI/CEGUIInputEvent.h>

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
	, _gui(0)
	, _console(0)
	, _editor(0)
	, _loading_screen(0)
	, _stats(0)
	, _console_memory_index(-1)	// Using -1 index to indicate not using memory
	, _running(true)	// TODO should this be true from the start?
	, _rendering(false)
{
	std::cout << vl::TRACE << "vl::Pipe::Pipe : name = " << _name << std::endl;

	_client->registerForUpdates();
}

vl::Pipe::~Pipe( void )
{
	std::cout << vl::TRACE << "vl::Pipe::~Pipe" << std::endl;

	// Some asserts for checking that the Pipe thread has been correctly shutdown
	assert( !_root );

	delete _scene_manager;
	delete _player;

	_client.reset();

	std::cout << vl::TRACE << "vl::Pipe::~Pipe : DONE" << std::endl;
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
vl::Pipe::sendCommand( std::string const &cmd )
{
	vl::cluster::Message msg( vl::cluster::MSG_COMMAND );
	// Write size and string and terminating character
	msg.write( cmd.size()+1 );
	msg.write( cmd.c_str(), cmd.size()+1 );

	sendMessageToMaster(&msg);
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

	// TODO these should be moved to a separate function
	_createOgre();

	vl::EnvSettings::Node node = getNodeConf();
	std::string msg = "Creating " + vl::to_string(node.getNWindows()) + " windows.";
	Ogre::LogManager::getSingleton().logMessage( msg, Ogre::LML_TRIVIAL );

	for( size_t i = 0; i < node.getNWindows(); ++i )
	{ _createWindow( node.getWindow(i) ); }

	_initGUI();

	while( true )
	{
		// Handle messages
		_handleMessages();
		if( !isRunning() )
		{ break; }

		// Middle of a rendering loop not sleeping
		if( !isRendering() )
		{
			// Process input events
			for( size_t i = 0; i < _windows.size(); ++i )
			{ _windows.at(i)->capture(); }

			// Sleep
			// TODO should sleep for 1ms
			boost::this_thread::sleep( boost::posix_time::milliseconds(0) );
		}

		// Send messages
		_sendEvents();
	}
}

void
vl::Pipe::printToConsole(std::string const &text, double time,
						 std::string const &type, vl::LOG_MESSAGE_LEVEL lvl)
{
	CEGUI::MultiColumnList *output = static_cast<CEGUI::MultiColumnList *>( _console->getChild("console/output") );
	assert( output );

	// Add time
	std::stringstream ss;
	ss << time;
	CEGUI::ListboxTextItem *item = new CEGUI::ListboxTextItem(ss.str());
	CEGUI::uint row = output->addRow(item, 1);

	// Set line number
	ss.str("");
	ss << row;
	item = new CEGUI::ListboxTextItem(ss.str());
	output->setItem(item, 0, row);

	// Add the text field
	CEGUI::String prefix;
	if( lvl == vl::LML_CRITICAL )
	{ prefix = "CRITICAL : "; }
	else if( lvl == vl::LML_TRIVIAL )
	{ prefix = "TRIVIAL : "; }

	item = new CEGUI::ListboxTextItem(prefix + CEGUI::String(text));
	// Save data type for filtering, HOW?
	if( type == "OUT" )
	{
		item->setTextColours(CEGUI::colour(0, 0.2, 0.4));
	}
	else if( type == "ERROR" )
	{
		item->setTextColours(CEGUI::colour(0.5, 0, 0));
	}
	else if( type == "PY_OUT" )
	{
		item->setTextColours(CEGUI::colour(0, 0.5, 0.5));
	}
	else if( type == "PY_ERROR" )
	{
		item->setTextColours(CEGUI::colour(0.5, 0.2, 0));
	}
	output->setItem(item, 2, row);
}


/// ------------------------- GECUI callbacks ----------------------------------
bool
vl::Pipe::onConsoleInputAccepted( CEGUI::EventArgs const &e )
{
	assert( _console );
	CEGUI::Editbox *input = static_cast<CEGUI::Editbox *>( _console->getChild("console/input") );
	assert( input );

	std::string command( input->getText().c_str() );

	if( command.size() > 0 )
	{
		input->setText("");

		if( *(command.end()-1) == ':' )
		{
			std::string str("Multi Line commands are not supported yet.");
			printToConsole(str, 0);
		}
		else
		{
			while( _console_memory.size() > 100 )
			{ _console_memory.pop_back(); }

			_console_memory.push_front(command);

			// TODO add support for time
			printToConsole(command, 0);

			sendCommand(command);
		}

		// Reset the memory index because the user has accepted the command
		_console_memory_index = -1;
		_console_last_command.clear();
	}

	return true;
}

bool
vl::Pipe::onConsoleInputKeyDown(const CEGUI::EventArgs& e)
{
	assert( _console );
	CEGUI::Editbox *input = static_cast<CEGUI::Editbox *>( _console->getChild("console/input") );
	assert( input );

	CEGUI::KeyEventArgs const &key = static_cast<CEGUI::KeyEventArgs const &>(e);
	if(key.scancode == CEGUI::Key::ArrowUp)
	{
		// Save the current user input when the list has not been scrolled
		if( _console_memory_index == -1 )
		{
			_console_last_command = input->getText().c_str();
		}

		++_console_memory_index;
		if( _console_memory_index >= _console_memory.size() )
		{ _console_memory_index = _console_memory.size()-1; }

		if( _console_memory_index > -1 )
		{
			std::string command = _console_memory.at(_console_memory_index);

			input->setText(command);
			input->setCaratIndex(input->getText().size());
		}

		return true;
	}
	else if(key.scancode == CEGUI::Key::ArrowDown)
	{
		--_console_memory_index;
		if( _console_memory_index < 0 )
		{
			_console_memory_index = -1;
			input->setText(_console_last_command);
		}
		else
		{
			std::string command = _console_memory.at(_console_memory_index);

			input->setText(command);
			input->setCaratIndex(input->getText().size());
		}

		return true;
	}

	return false;
}

bool
vl::Pipe::onConsoleShow(const CEGUI::EventArgs& e)
{
	assert( _console );
	CEGUI::Editbox *input = static_cast<CEGUI::Editbox *>( _console->getChild("console/input") );
	assert( input );

	input->activate();

	return true;
}

/// ------------------------ Protected -----------------------------------------
void
vl::Pipe::_reloadProjects( vl::Settings const &set )
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

void
vl::Pipe::_initGUI(void )
{
	std::string message( "vl::Pipe::_initGUI" );
	Ogre::LogManager::getSingleton().logMessage(message, Ogre::LML_TRIVIAL);

	// TODO support for multiple windows

	// TODO this should be cleanup, should work with any codec or parser...
#ifdef VL_WIN32
	CEGUI::System::setDefaultImageCodecName( "SILLYImageCodec" );
	CEGUI::System::setDefaultXMLParserName( "ExpatParser" );
#endif
	assert( _windows.size() > 0);
	Ogre::RenderWindow *win = _windows.at(0)->getRenderWindow();
	assert(win);

	CEGUI::OgreRenderer& myRenderer = CEGUI::OgreRenderer::create(*win);
	CEGUI::System::create(myRenderer);
}

void
vl::Pipe::_initGUIResources( vl::Settings const &settings )
{
	std::string message( "vl::Pipe::_initGUIResources" );
	Ogre::LogManager::getSingleton().logMessage(message, Ogre::LML_TRIVIAL);

	assert( !settings.empty() );
	// Find global project and add those resources as default
	std::string projName("global");
	if( settings.hasProject(projName) )
	{
		vl::ProjSettings const &proj = settings.findProject(projName);
		message = "Found " + projName + " project";
		Ogre::LogManager::getSingleton().logMessage(message);

		fs::path gui_path = fs::path(settings.getDir(proj)) / "gui";
		if( fs::is_directory( gui_path ) )
		{
			_addGUIResourceGroup( "schemes", gui_path / "schemes/" );
			_addGUIResourceGroup( "imagesets", gui_path / "imagesets/" );
			_addGUIResourceGroup( "fonts", gui_path / "fonts/" );
			_addGUIResourceGroup( "layouts", gui_path / "layouts/" );
			_addGUIResourceGroup( "looknfeels", gui_path / "looknfeel/" );
			_addGUIResourceGroup( "lua_scripts", gui_path / "lua_scripts/" );

			// set the default resource groups to be used
			CEGUI::Imageset::setDefaultResourceGroup("imagesets");
			CEGUI::Font::setDefaultResourceGroup("fonts");
			CEGUI::Scheme::setDefaultResourceGroup("schemes");
			CEGUI::WidgetLookManager::setDefaultResourceGroup("looknfeels");
			CEGUI::WindowManager::setDefaultResourceGroup("layouts");
			CEGUI::ScriptModule::setDefaultResourceGroup("lua_scripts");
		}
		else
		{
			message = projName + " Does not have gui resource directory.";
			Ogre::LogManager::getSingleton().logMessage(message, Ogre::LML_CRITICAL);
		}
	}
	else
	{
		message = projName + " NOT found. There will be no GUI.";
		Ogre::LogManager::getSingleton().logMessage(message, Ogre::LML_CRITICAL);
	}

	// Find editor project and add those resources to groupd editor
	projName = "editor";
	if( settings.hasProject(projName) )
	{
		message = "Found editor project";
		Ogre::LogManager::getSingleton().logMessage(message);
		settings.findProject(projName);

		// TODO really load the editor resources
	}
	else
	{
		message = projName + " NOT found. There will be no Editor.";
		Ogre::LogManager::getSingleton().logMessage(message, Ogre::LML_CRITICAL);
	}

	// TODO Add project and add those resources to group projectName
}

void
vl::Pipe::_addGUIResourceGroup( std::string const &name, fs::path const &path )
{
	CEGUI::DefaultResourceProvider *rp = static_cast<CEGUI::DefaultResourceProvider *>
		(CEGUI::System::getSingleton().getResourceProvider());
	assert(rp);

	if( fs::is_directory( path ) )
	{
		std::string message = "GUI resource " + name + " added "
			" with path " + path.file_string() + ".";
		Ogre::LogManager::getSingleton().logMessage(message);

		rp->setResourceGroupDirectory( name, path.file_string() );
	}
	else
	{
		std::string message = "GUI resource " + name + " couldn't be added "
			"because path " + path.file_string() + " does NOT exist.";
		Ogre::LogManager::getSingleton().logMessage(message);
	}
}


void
vl::Pipe::_createGUI(void )
{
	std::string message( "vl::Pipe::_createGUI" );
	Ogre::LogManager::getSingleton().logMessage(message);

	// Load default data files used for the GUI
	CEGUI::SchemeManager::getSingleton().create( "TaharezLook.scheme" );
	CEGUI::FontManager::getSingleton().create( "DejaVuSans-7.font" );
	CEGUI::FontManager::getSingleton().create( "DejaVuSans-8.font" );
	CEGUI::FontManager::getSingleton().create( "DejaVuSans-10.font" );
	CEGUI::FontManager::getSingleton().create( "DejaVuSans-9.font" );
	CEGUI::FontManager::getSingleton().create( "DejaVuSans-6.font" );
	CEGUI::System::getSingleton().setDefaultMouseCursor( "TaharezLook", "MouseArrow" );

	// Create the GUI windows
	assert( _windows.size() > 0 );
	Window *win = _windows.at(0);

	CEGUI::Window *myRoot = CEGUI::WindowManager::getSingleton().loadWindowLayout( "editor.layout" );
	CEGUI::System::getSingleton().setGUISheet( myRoot );
	_editor = myRoot->getChild("editor");

	_console = CEGUI::WindowManager::getSingleton().loadWindowLayout( "console.layout" );
	myRoot->addChildWindow(_console);

	_loading_screen = CEGUI::WindowManager::getSingleton().loadWindowLayout( "loading_screen.layout" );
	myRoot->addChildWindow(_loading_screen);
	_loading_screen->hide();

	_stats = CEGUI::WindowManager::getSingleton().loadWindowLayout( "stats.layout" );
	myRoot->addChildWindow(_stats);
	_stats->hide();

	win->createGUIWindow();

	/// Subscripe to events
	std::cout << "Subcribing to events." << std::endl;
	_console->subscribeEvent(CEGUI::FrameWindow::EventShown, CEGUI::Event::Subscriber(&vl::Pipe::onConsoleShow, this));

	CEGUI::MultiLineEditbox *output = static_cast<CEGUI::MultiLineEditbox *>( _console->getChild("console/output") );
	assert(output);
	assert(output->getVertScrollbar());
	output->getVertScrollbar()->setEndLockEnabled(true);

	CEGUI::Editbox *input = static_cast<CEGUI::Editbox *>( _console->getChild("console/input") );
	assert(input);
	input->subscribeEvent(CEGUI::Editbox::EventTextAccepted, CEGUI::Event::Subscriber(&vl::Pipe::onConsoleInputAccepted, this));
	input->subscribeEvent(CEGUI::Editbox::EventKeyDown, CEGUI::Event::Subscriber(&vl::Pipe::onConsoleInputKeyDown, this));

	CEGUI::MenuItem *item = static_cast<CEGUI::MenuItem *>( _editor->getChildRecursive("editor/newItem") );
	assert( item );
	item->subscribeEvent(CEGUI::MenuItem::EventClicked, CEGUI::Event::Subscriber(&vl::Window::onNewClicked, win));

	item = static_cast<CEGUI::MenuItem *>( _editor->getChildRecursive("editor/openItem") );
	assert( item );
	item->subscribeEvent(CEGUI::MenuItem::EventClicked, CEGUI::Event::Subscriber(&vl::Window::onOpenClicked, win));

	item = static_cast<CEGUI::MenuItem *>( _editor->getChildRecursive("editor/saveItem") );
	assert( item );
	item->subscribeEvent(CEGUI::MenuItem::EventClicked, CEGUI::Event::Subscriber(&vl::Window::onSaveClicked, win));

	item = static_cast<CEGUI::MenuItem *>( _editor->getChildRecursive("editor/quitItem") );
	assert( item );
	item->subscribeEvent(CEGUI::MenuItem::EventClicked, CEGUI::Event::Subscriber(&vl::Window::onQuitClicked, win));

	item = static_cast<CEGUI::MenuItem *>( _editor->getChildRecursive("editor/resetItem") );
	assert( item );
	item->subscribeEvent(CEGUI::MenuItem::EventClicked, CEGUI::Event::Subscriber(&vl::Window::onResetClicked, win));

	item = static_cast<CEGUI::MenuItem *>( _editor->getChildRecursive("editor/importSceneItem") );
	assert( item );
	item->subscribeEvent(CEGUI::MenuItem::EventClicked, CEGUI::Event::Subscriber(&vl::Window::onImportSceneClicked, win));

	item = static_cast<CEGUI::MenuItem *>( _editor->getChildRecursive("editor/reloadScenes") );
	assert( item );
	item->subscribeEvent(CEGUI::MenuItem::EventClicked, CEGUI::Event::Subscriber(&vl::Window::onReloadScenesClicked, win));

	item = static_cast<CEGUI::MenuItem *>( _editor->getChildRecursive("editor/addScriptItem") );
	assert( item );
	item->subscribeEvent(CEGUI::MenuItem::EventClicked, CEGUI::Event::Subscriber(&vl::Window::onAddScriptClicked, win));

	item = static_cast<CEGUI::MenuItem *>( _editor->getChildRecursive("editor/newScriptItem") );
	assert( item );
	item->subscribeEvent(CEGUI::MenuItem::EventClicked, CEGUI::Event::Subscriber(&vl::Window::onNewScriptClicked, win));

	item = static_cast<CEGUI::MenuItem *>( _editor->getChildRecursive("editor/reloadScripts") );
	assert( item );
	item->subscribeEvent(CEGUI::MenuItem::EventClicked, CEGUI::Event::Subscriber(&vl::Window::onReloadScriptsClicked, win));


	CEGUI::Checkbox *checkBox = static_cast<CEGUI::Checkbox *>( _editor->getChildRecursive("editor/showAxes") );
	assert( checkBox );
	checkBox->subscribeEvent(CEGUI::Checkbox::EventCheckStateChanged, CEGUI::Event::Subscriber(&vl::Window::onShowAxisChanged, win));

	checkBox = static_cast<CEGUI::Checkbox *>( _editor->getChildRecursive("editor/showNames") );
	assert( checkBox );
	checkBox->subscribeEvent(CEGUI::Checkbox::EventCheckStateChanged, CEGUI::Event::Subscriber(&vl::Window::onShowNamesChanged, win));

	checkBox = static_cast<CEGUI::Checkbox *>( _editor->getChildRecursive("editor/showJoints") );
	assert( checkBox );
	checkBox->subscribeEvent(CEGUI::Checkbox::EventCheckStateChanged, CEGUI::Event::Subscriber(&vl::Window::onShowJointsChanged, win));

	// Request output updates for the console
	_client->registerForOutput();

	// TODO support for multiple windows
	// at the moment every window will get the same GUI window layout
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
	_root.reset( new vl::ogre::Root(_env->getLogLevel()) );
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
	assert( _client );

	_client->mainloop();
	while( _client->messages() )
	{
		vl::cluster::Message *msg = _client->popMessage();
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
			std::cout << vl::TRACE << "vl::Pipe::_handleMessage : MSG_ENVIRONMENT message" << std::endl;
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
			std::cout << vl::TRACE << "vl::Pipe::_handleMessage : MSG_PROJECT message" << std::endl;
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
			_initGUIResources(projects);
			_createGUI();
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
			_rendering = true;
		}
		break;

		case vl::cluster::MSG_DRAW :
		{
			_client->sendAck( vl::cluster::MSG_DRAW );
			_draw();
			_swap();
			_rendering = false;
		}
		break;

		case vl::cluster::MSG_SWAP :
		{
			_client->sendAck( vl::cluster::MSG_SWAP );
		}
		break;

		case vl::cluster::MSG_PRINT :
		{
			_handlePrintMsg(msg);
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

		switch( type )
		{
			case OBJ_PLAYER :
			{
				// TODO support multiple players
				assert( !_player );
				// TODO fix the constructor
				_player = new vl::Player;
				mapObject( _player, id );
				break;
			}
			case OBJ_GUI :
			{
				assert( !_gui );
				_gui = new vl::gui::GUI( this, id );
				_gui->setEditor( vl::gui::WindowRefPtr( new vl::gui::Window(_editor) ) );
				_gui->setConsole( vl::gui::WindowRefPtr( new vl::gui::Window(_console) ) );
				_gui->setLoadingScreen( vl::gui::WindowRefPtr( new vl::gui::Window(_loading_screen) ) );
				_gui->setStats( vl::gui::WindowRefPtr( new vl::gui::Window(_stats) ) );
				break;
			}
			case OBJ_SCENE_MANAGER :
			{
				// TODO support multiple SceneManagers
				assert( !_scene_manager );
				_scene_manager = new SceneManager( this, id );
				assert( _ogre_sm );
				_scene_manager->setSceneManager( _ogre_sm );
				break;
			}
			case OBJ_SCENE_NODE :
			{
				assert( _scene_manager );
				_scene_manager->createSceneNode( "", id );
				break;
			}
			case OBJ_ENTITY :
			{
				assert( _scene_manager );
				_scene_manager->createEntity( "", vl::PF_NONE, id );
				break;
			}
			default :
				// TODO Might happen something unexpected so for now just kill the program
				assert( false );
		}
	}
}

void
vl::Pipe::_handleUpdateMsg( vl::cluster::Message* msg )
{
	// Read the IDs in the message and call pack on mapped objects
	// based on thoses
	/// @TODO multiple update messages in the same frame,
	/// only the most recent should be used.
	while( msg->size() > 0 )
	{
		vl::cluster::ObjectData data;
		data.copyFromMessage(msg);
		// Pushing back will create copies which is unnecessary
		_objects.push_back(data);
	}
}

void
vl::Pipe::_handlePrintMsg( vl::cluster::Message *msg )
{
	assert( msg->getType() == vl::cluster::MSG_PRINT );
	size_t msgs;
	msg->read(msgs);
	while(msgs > 0)
	{
		std::string type;
		msg->read(type);
		double time;
		msg->read(time);
		std::string str;
		msg->read(str);
		vl::LOG_MESSAGE_LEVEL lvl;
		msg->read(lvl);

		printToConsole(str, time, type, lvl);

		msgs--;
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
	// TODO these should be moved to player using functors
	if( _player )
	{
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

		for( size_t i = 0; i < _windows.size(); ++i )
		{ _windows.at(i)->setIPD( _player->getIPD() ); }

		// Take a screenshot
		if( _player->getScreenshotVersion() > _screenshot_num )
		{
			_takeScreenshot();

			_screenshot_num = _player->getScreenshotVersion();
		}
	}
}

void
vl::Pipe::_draw( void )
{
	Ogre::WindowEventUtilities::messagePump();
	for( size_t i = 0; i < _windows.size(); ++i )
	{ _windows.at(i)->draw(); }

	if( guiShown() )
	{
		CEGUI::System::getSingleton().renderGUI();
	}
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
	std::cout << vl::TRACE << "vl::Pipe::_createWindow : " << winConf.name << std::endl;

	vl::Window *window = new vl::Window( winConf.name, this );
	assert( window );
	_windows.push_back(window);
}

void
vl::Pipe::_shutdown( void )
{
	std::cout << vl::TRACE << "vl::Pipe::_shutdown" << std::endl;

	std::vector<Window *>::iterator iter;
	for( iter = _windows.begin(); iter != _windows.end(); ++iter )
	{ delete *iter; }
	std::cout << vl::TRACE << "vl::Pipe::~Pipe : windows deleted" << std::endl;

	_root->getNative()->destroySceneManager( _ogre_sm );

	// Info
	std::string message("Cleaning out OGRE");
	Ogre::LogManager::getSingleton().logMessage( message );
	_root.reset();

	_running = false;

	std::cout << vl::TRACE << "vl::Pipe::_shutdown : DONE" << std::endl;
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
	std::cout << vl::TRACE << "PipeThread Thread entered." << std::endl;

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
		std::clog << "VL Exception : "<<   boost::diagnostic_information<>(e)
			<< std::endl;
	}
	catch( Ogre::Exception const &e)
	{
		std::clog << "Ogre Exception: " << e.what() << std::endl;
	}
	catch( std::exception const &e )
	{
		std::clog << "STD Exception: " << e.what() << std::endl;
	}
	catch( ... )
	{
		std::clog << "An exception of unknow type occured." << std::endl;
	}

	delete pipe;
}
