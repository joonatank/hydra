/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-01
 *	@file renderer.cpp
 *
 */

// Interface
#include "renderer.hpp"

// Necessary for printing error messages from exceptions
#include "base/exceptions.hpp"

#include "window.hpp"
#include "base/string_utils.hpp"
#include "base/sleep.hpp"
#include "distrib_settings.hpp"

#include "gui/gui.hpp"
#include "gui/gui_window.hpp"

#include "logger.hpp"

#include <OGRE/OgreWindowEventUtilities.h>

/// ------------------------- Public -------------------------------------------
// TODO should probably copy the env settings and not store the reference
vl::Renderer::Renderer(std::string const &name)
	: _name(name)
	, _ogre_sm(0)
	, _camera(0)
	, _scene_manager(0)
	, _player(0)
	, _screenshot_num(0)
	, _send_message_cb(0)
	, _n_log_messages(0)
{
	std::cout << vl::TRACE << "vl::Renderer::Renderer : name = " << _name << std::endl;
}

vl::Renderer::~Renderer(void)
{
	std::cout << vl::TRACE << "vl::Renderer::~Renderer" << std::endl;

	std::vector<Window *>::iterator iter;
	for( iter = _windows.begin(); iter != _windows.end(); ++iter )
	{ delete *iter; }
	_windows.clear();

	// Shouldn't be necessary anymore, if _root handles destruction cleanly
	if( _root && _ogre_sm )
	{ _root->getNative()->destroySceneManager( _ogre_sm ); }
	_ogre_sm = 0;

	_root.reset();

	delete _scene_manager;
	delete _player;

	std::cout << vl::TRACE << "vl::Renderer::~Renderer : DONE" << std::endl;
}

void
vl::Renderer::init(vl::EnvSettingsRefPtr env)
{
	std::cout << vl::TRACE << "vl::Renderer::init" << std::endl;

	assert(env);
	// Single init allowed
	assert(!_env);
	_env = env;

	_createOgre(env);

	vl::EnvSettings::Node node = getNodeConf();
	std::string msg = "Creating " + vl::to_string(node.getNWindows()) + " windows.";
	Ogre::LogManager::getSingleton().logMessage( msg, Ogre::LML_TRIVIAL );

	for( size_t i = 0; i < node.getNWindows(); ++i )
	{ _createWindow( node.getWindow(i) ); }
}

vl::EnvSettings::Node
vl::Renderer::getNodeConf( void )
{
	assert(_env);

	vl::EnvSettings::Node node;
	if( getName() == _env->getMaster().name )
	{ node = _env->getMaster(); }
	else
	{ node = _env->findSlave( getName() ); }

	assert( !node.empty() );
	return node;
}

vl::EnvSettings::Window
vl::Renderer::getWindowConf( std::string const &window_name )
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
vl::Renderer::sendEvent( vl::cluster::EventData const &event )
{
	// Add to event stack for sending them at once in one message to the Master
	_events.push_back(event);
}

void
vl::Renderer::sendCommand( std::string const &cmd )
{
	vl::cluster::Message msg( vl::cluster::MSG_COMMAND, 0, vl::time() );
	// Write size and string and terminating character
	msg.write( cmd.size()+1 );
	msg.write( cmd.c_str(), cmd.size()+1 );

	// Callback
	assert(_send_message_cb);
	(*_send_message_cb)(msg);
}

void
vl::Renderer::capture(void)
{
	// Here we should wait for the EnvSettings from master
	// TODO we should have a wait for Message function
	if( !_env )
	{
		return;
	}

	// Process input events
	for( size_t i = 0; i < _windows.size(); ++i )
	{ _windows.at(i)->capture(); }

	// Send messages
	_sendEvents();
}

bool 
vl::Renderer::guiShown(void) const
{
	if( !_gui )
	{ return false; }
	return _gui->shown();
}

void
vl::Renderer::printToConsole(std::string const &text, double time,
						 std::string const &type, vl::LOG_MESSAGE_LEVEL lvl)
{
	_gui->getConsole()->printTo(text, time, type, lvl);
}


void
vl::Renderer::draw(void)
{
	Ogre::WindowEventUtilities::messagePump();
	
	// @todo move the callback notifiying to Root
	_root->getNative()->_fireFrameStarted();

	if(_scene_manager)
	{ _scene_manager->_notifyFrameStart(); }

	for( size_t i = 0; i < _windows.size(); ++i )
	{ _windows.at(i)->draw(); }

	if(_scene_manager)
	{ _scene_manager->_notifyFrameEnd(); }

	_root->getNative()->_fireFrameEnded();
}

void
vl::Renderer::swap(void)
{
	for( size_t i = 0; i < _windows.size(); ++i )
	{ _windows.at(i)->swap(); }
}

void
vl::Renderer::setProject(vl::cluster::Message& msg)
{
	assert(msg.getType() == vl::cluster::MSG_PROJECT);
	
	std::cout << vl::TRACE << "vl::Renderer::setProject" << std::endl;

	// TODO
	// Problematic because the Project config should be
	// updatable during the application run
	// And this one will create them anew, so that we need to invalidate
	// the scene and reload everything
	// NOTE
	// Combining the project configurations is not done automatically
	// so they either need special structure or we need to iterate over
	// all of them always.
	/// @todo replace ByteDataStream with MessageStream, reduces copying
	vl::SettingsByteData data;
	data.copyFromMessage(&msg);
	vl::cluster::ByteDataStream stream(&data);
	stream >> _settings;

	_initialiseResources(_settings);
}

void
vl::Renderer::initScene(vl::cluster::Message& msg)
{
	/// @todo change to use a custom type
	assert(msg.getType() == vl::cluster::MSG_SG_UPDATE);
	
	std::cout << vl::TRACE << "vl::Renderer::initScene" << std::endl;
	
	updateScene(msg);
}

void
vl::Renderer::createSceneObjects(vl::cluster::Message& msg)
{
	assert(msg.getType() == vl::cluster::MSG_SG_CREATE);
	
	size_t size;
	assert( msg.size() >= sizeof(size) );

	msg.read( size );
	for( size_t i = 0; i < size; ++i )
	{
		assert( msg.size() > 0 );
		OBJ_TYPE type;
		uint64_t id;

		msg.read(type);
		msg.read(id);

		switch( type )
		{
			case OBJ_PLAYER :
			{
				std::cout << vl::TRACE << "Creating Player with ID : " << id << std::endl;
				// @todo replace asserts with throwing
				assert(_scene_manager);
				vl::Player *player = new vl::Player(_scene_manager);
				mapObject(player, id);
				
				// Only single instances are supported for now
				assert(!_player && player);
				_player = player;
			}
			break;

			case OBJ_GUI :
			{
				std::cout << vl::TRACE << "Creating GUI with id " << id << std::endl;
				
				// Only creating GUI on the master for now
				// @todo add support for selecting the window
				if( getName() == _env->getMaster().name )
				{
					std::cout << vl::TRACE << "Creating GUI" << std::endl;
					// Do not create the GUI multiple times
					assert(!_gui);
					_gui.reset(new vl::gui::GUI(this, id, new RendererCommandCallback(this)));
					assert(_windows.size() > 0);
					_gui->initGUI(_windows.at(0));
					assert(!_settings.empty());

					_gui->initGUIResources(_settings);
					_gui->createGUI();

					// Request output updates for the console
					if(logEnabled())
					{
						assert( _send_message_cb );
						vl::cluster::Message msg(vl::cluster::MSG_REG_OUTPUT, 0, vl::time());
						(*_send_message_cb)(msg);
					}
				}
				else
				{
					// Slaves need to ignore the GUI updates
					_ignored_distributed_objects.push_back(id);
				}
			}
			break;

			case OBJ_SCENE_MANAGER :
			{
				std::cout << vl::TRACE << "Creating SceneManager" << std::endl;
				// TODO support multiple SceneManagers
				assert(!_scene_manager);
				assert(!_ogre_sm);
				assert(_mesh_manager);
				// TODO should pass the _ogre_sm to there also or vl::Root as creator
				_ogre_sm = _createOgreSceneManager(_root, "SceneManager");
				_scene_manager = new SceneManager(this, id, _ogre_sm, _mesh_manager);
				std::clog << "SceneManager created." << std::endl;
			}
			break;

			case OBJ_SCENE_NODE :
			{
				assert(_scene_manager);
				_scene_manager->_createSceneNode(id);
			}
			break;

			/// @todo MovableObjects type should be divided into two one for
			/// the movable object type and other one dynamic so more movable objects
			/// can be created with ease.
			default :
				assert( _scene_manager );
				_scene_manager->_createMovableObject(type, id);
				break;
		}
	}
}

void
vl::Renderer::updateScene(vl::cluster::Message& msg)
{
	assert(msg.getType() == vl::cluster::MSG_SG_UPDATE);

	// Read the IDs in the message and call pack on mapped objects
	// based on thoses
	/// @TODO multiple update messages in the same frame,
	/// only the most recent should be used.
	while( msg.size() > 0 )
	{
		vl::cluster::ObjectData data;
		data.copyFromMessage(&msg);
		// Pushing back will create copies which is unnecessary
		_objects.push_back(data);
	}

	_syncData();
	_updateDistribData();
}

void
vl::Renderer::print(vl::cluster::Message& msg)
{
	assert( msg.getType() == vl::cluster::MSG_PRINT );
	size_t msgs;
	msg.read(msgs);
	while(msgs > 0)
	{
		std::string type;
		msg.read(type);
		double time;
		msg.read(time);
		std::string str;
		msg.read(str);
		vl::LOG_MESSAGE_LEVEL lvl;
		msg.read(lvl);

		printToConsole(str, time, type, lvl);

		msgs--;
	}
}

void 
vl::Renderer::setSendMessageCB(vl::MsgCallback *cb)
{
	// Only single instances are supported for now
	assert(!_send_message_cb && cb);
	_send_message_cb = cb;
}

/// ----------------------- Log Receiver overrides ---------------------------
bool 
vl::Renderer::logEnabled(void) const
{
	if(_gui && _gui->getConsole() && _gui->getConsole()->wantsLogging())
	{ return true; }

	return false;
}

void 
vl::Renderer::logMessage(vl::LogMessage const &msg)
{
	printToConsole(msg.message, msg.time, msg.type, msg.level);
	++_n_log_messages;
}

uint32_t 
vl::Renderer::nLoggedMessages(void) const
{ return _n_log_messages; }

/// ------------------------ Protected -----------------------------------------


/// Ogre helpers
void
vl::Renderer::_createOgre(vl::EnvSettingsRefPtr env)
{
	std::cout << vl::TRACE << "vl::Renderer::_createOgre" << std::endl;

	assert(env);

	// A hack to set display on Linux
	// Should really pass the display number to Ogre Window creation
	// but this requires modification of Ogre library
	// Also this only sets the display using the first window config
	// can't do much about it because when Ogre initialises the rendering
	// system it grabs the current display.
#ifndef _WIN32
	if(getNodeConf().getNWindows() > 0 && getNodeConf().getWindow(0).n_display > -1)
	{
		std::stringstream ss_disp;
		// format :0.x where x is the display number
		ss_disp << ":0." << getNodeConf().getWindow(0).n_display;
		::setenv("DISPLAY", ss_disp.str().c_str(), 1);
	}
	
	char *disp_env = ::getenv("DISPLAY");
	std::cout << "DISPLAY environment variable = " << disp_env << std::endl;
#endif

	// TODO the project name should be used instead of the hydra for all
	// problem is that the project name is not known at this point
	// so we should use a tmp file and then move it.
	_root.reset( new vl::ogre::Root(env->getLogLevel()) );
	// Initialise ogre
	_root->createRenderSystem();
}

void
vl::Renderer::_initialiseResources( vl::Settings const &set )
{
	assert(_root);

	// Add resources
	_root->addResource( set.getProjectDir() );
	for( size_t i = 0; i < set.getAuxDirectories().size(); ++i )
	{
		_root->addResource( set.getAuxDirectories().at(i) );
	}

	std::string msg("Setting up the resources.");
	Ogre::LogManager::getSingleton().logMessage( msg, Ogre::LML_TRIVIAL );

	_root->setupResources();
	_root->loadResources();
}

Ogre::SceneManager *
vl::Renderer::_createOgreSceneManager(vl::ogre::RootRefPtr root, std::string const &name)
{
	assert(root);
	Ogre::SceneManager *sm = _root->createSceneManager(name);

	/// These can not be moved to SceneManager at least not yet
	/// because they need the RenderSystem capabilities.
	/// @todo this should be user configurable (if the hardware supports it)
	/// @todo the number of textures (four at the moment) should be user configurable
	if (root->getNative()->getRenderSystem()->getCapabilities()->hasCapability(Ogre::RSC_HWRENDER_TO_TEXTURE))
	{
		std::cout << "Using 1024 x 1024 shadow textures." << std::endl;
		sm->setShadowTextureSettings(1024, 4);
	}
	else
	{
		/// @todo this doesn't work on Windows with size < (512,512)
		/// should check the window size and select the largest
		/// possible shadow texture based on that.
		std::cout << "Using 512 x 512 shadow textures." << std::endl;
		sm->setShadowTextureSettings(512, 4);
	}

	return sm;
}


/// Distribution helpers
void
vl::Renderer::_syncData(void)
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
		// Skip objects that this slave should not update
		if(std::find(_ignored_distributed_objects.begin(), _ignored_distributed_objects.end(),
			iter->getId()) == _ignored_distributed_objects.end())
		{
			vl::cluster::ByteDataStream stream = iter->getStream();
			vl::Distributed *obj = findMappedObject( iter->getId() );
			if( obj )
			{
				obj->unpack(stream);
			}
			else
			{
				std::cout << vl::CRITICAL << "No ID " << iter->getId() << " found in mapped objects."
					<< std::endl;
				tmp.push_back( *iter );
			}
		}
	}

	_objects = tmp;
}

void
vl::Renderer::_updateDistribData( void )
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
			assert(_scene_manager);
			if( _scene_manager->hasCamera( cam_name ) )
			{
				// Tell the Windows to change cameras
				_camera = _scene_manager->getCamera( _active_camera_name );
				assert( !_windows.empty() );
				for( size_t i = 0; i < _windows.size(); ++i )
				{ _windows.at(i)->setCamera( _camera ); }
			}
			else
			{
				// Throws because this should be already checked when the camera
				// is set to player.
				std::string error_msg(std::string("New camera name set, but NO \"")
					+ cam_name + std::string("\" camera found"));
				BOOST_THROW_EXCEPTION(vl::item_not_found() << vl::desc(error_msg));
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
vl::Renderer::_sendEvents( void )
{
	if( !_events.empty() )
	{
		vl::cluster::Message msg( vl::cluster::MSG_INPUT, 0, vl::time() );
		std::vector<vl::cluster::EventData>::iterator iter;
		for( iter = _events.begin(); iter != _events.end(); ++iter )
		{
			iter->copyToMessage(&msg);
		}
		_events.clear();

		assert(_send_message_cb);
		(*_send_message_cb)(msg);
	}
}

void
vl::Renderer::_createWindow( vl::EnvSettings::Window const &winConf )
{
	std::cout << vl::TRACE << "vl::Renderer::_createWindow : " << winConf.name << std::endl;

	vl::Window *window = new vl::Window( winConf.name, this );
	window->setCamera(_camera);
	_windows.push_back(window);
}

void
vl::Renderer::_takeScreenshot( void )
{
	std::string prefix( "screenshot_" );
	std::string suffix = ".png";

	// Tell the Window to take a screenshot
	for( size_t i = 0; i < _windows.size(); ++i )
	{ _windows.at(i)->takeScreenshot( prefix, suffix ); }
}

