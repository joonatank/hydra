/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011/10 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-01
 *	@file renderer.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
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
#include "material_manager.hpp"

#include "gui/gui.hpp"
#include "gui/gui_window.hpp"
#include "gui/console.hpp"

#include "logger.hpp"

// SkyX updates
#include "camera.hpp"
#include "scene_manager.hpp"

// Necessary for checking materials after deserialize
#include "material.hpp"

// Necessary for message pump
#include <OGRE/OgreWindowEventUtilities.h>

#include <boost/bind.hpp>

/// ------------------------- Public -------------------------------------------
vl::Renderer::Renderer(std::string const &name)
	: _name(name)
	, _ogre_sm(0)
	, _scene_manager(0)
	, _player(0)
	, _screenshot_num(0)
	, _n_log_messages(0)
	, _enable_debug_overlay(false)
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

	// Necessary because this holds Ogre resources.
	_material_manager.reset();

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
vl::Renderer::init(vl::config::EnvSettingsRefPtr env)
{
	std::cout << vl::TRACE << "vl::Renderer::init" << std::endl;

	assert(env);
	// Single init allowed
	assert(!_env);
	_env = env;

	_createOgre(env);

	vl::config::Node const &node = getNodeConf();
	if(!node.empty())
	{ 
		std::cout << vl::TRACE << "Creating " << node.getNWindows() << " windows." << std::endl;

		for( size_t i = 0; i < node.getNWindows(); ++i )
		{ createWindow( node.getWindow(i) ); }
	}
	// Hack to handle Ogre's depency on Window creation before the use of
	// the RenderingEngine
	else
	{
		// @todo should destroy the window after the init
		createWindow(vl::config::Window("dummy", vl::config::Channel(), 400, 320, 0, 0));
	}
}

vl::config::Node const &
vl::Renderer::getNodeConf(void) const
{
	if(!_env)
	{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("No environment configuration.")); }

	if(getName() == _env->getMaster().name)
	{ return _env->getMaster(); }
	else
	{ return _env->findSlave( getName() ); }
}

vl::config::Window const &
vl::Renderer::getWindowConf(std::string const &window_name) const
{
	vl::config::Node const &node = getNodeConf();

	// TODO add real errors
	if( node.getNWindows() == 0 )
	{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Node has no windows.")); }

	for( size_t i = 0; i < node.getNWindows(); ++i )
	{
		if( node.getWindow(i).name == window_name )
		{ return node.getWindow(i); }
	}

	BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("No such Window"));
}

void
vl::Renderer::sendEvent(vl::cluster::EventData const &event)
{
	_event_signal(event);
}

void
vl::Renderer::sendCommand( std::string const &cmd )
{
	_command_signal(cmd);
}

void
vl::Renderer::capture(void)
{
	// Process input events
	for( size_t i = 0; i < _windows.size(); ++i )
	{ _windows.at(i)->capture(); }
}

bool 
vl::Renderer::guiShown(void) const
{
	if( !_gui )
	{ return false; }
	return _gui->isVisible();
}

void
vl::Renderer::printToConsole(std::string const &text, double time,
						 std::string const &type, vl::LOG_MESSAGE_LEVEL lvl)
{
	if(_gui &&_gui->getConsole())
	{ _gui->getConsole()->printTo(text, time, type, lvl); }
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

	// Hack to update Sky
	if(_windows.size() > 0 && _windows.at(0)->getPlayer().getCamera()
		&& _scene_manager && _scene_manager->getSkySimulator())
	{
		_scene_manager->getSkySimulator()->notifyCameraRender(_windows.at(0)->getPlayer().getCamera());
	}

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
vl::Renderer::setProject(vl::Settings const &settings)
{
	std::cout << vl::TRACE << "vl::Renderer::setProject" << std::endl;

	_settings = settings;
	
	assert(_root);

	// @todo this doesn't allow resetting the resources

	std::vector<std::string> paths;

	// Add resources
	if(!settings.getProjectDir().empty())
	{ paths.push_back(settings.getProjectDir()); }
	for(size_t i = 0; i < settings.getAuxDirectories().size(); ++i)
	{
		if(!settings.getAuxDirectories().at(i).empty())
		{ paths.push_back(settings.getAuxDirectories().at(i)); }
	}

	std::clog << "Setting up the Ogre resources." << std::endl;

	std::clog << "Adding paths to resources : " << std::endl;
	for(size_t i = 0; i < paths.size(); ++i)
	{
		std::clog << "\t" << paths.at(i) << std::endl;
	}

	// remove all old resources
	_root->removeResources();
	// add all resources
	_root->setupResources(paths);
	// initialise them
	_root->loadResources();

	// @todo reset CEGUI resources also

	// Remove cameras and wait for them to be reset
	for(size_t i = 0; i < _windows.size(); ++i)
	{
		_windows.at(i)->setCamera(0);
		_windows.at(i)->resetStatistics();
	}
}

void
vl::Renderer::createSceneObjects(vl::cluster::Message& msg)
{
	std::cout << vl::TRACE << "vl::Renderer::createSceneObjects" << std::endl;

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
				if(!_scene_manager)
				{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("No SceneManager")); }

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
					if(_gui)
					{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("GUI already created")); }

					// @todo fix callback with a signal
					_gui.reset(new vl::gui::GUI(this, id));
					assert(_windows.size() > 0);
					assert(_windows.at(0)->getViewport());
					_gui->initGUI(_windows.at(0)->getViewport());
				}
				else
				{
					// Slaves need to ignore the GUI updates
					_ignored_distributed_objects.push_back(id);
				}
			}
			break;

			case OBJ_GUI_CONSOLE :
			{
				// GUI objects are only used by master
				if( getName() == _env->getMaster().name )
				{
					if(!_gui)
					{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("NO GUI when trying to create GUI::Window.")); }
					_gui->createWindow(type, id);
					if(type == OBJ_GUI_CONSOLE)
					{
						_gui->getConsole()->addCommandListener(boost::bind(&Renderer::sendCommand, this, _1));
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
				std::cout << vl::TRACE << "Renderer : Creating SceneManager" << std::endl;
				// TODO support multiple SceneManagers
				if(_scene_manager || _ogre_sm)
				{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("SceneManager already created.")); }
				if(!_mesh_manager)
				{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("No MeshManager.")); }

				// TODO should pass the _ogre_sm to there also or vl::Root as creator
				_ogre_sm = _createOgreSceneManager(_root, "SceneManager");
				_scene_manager = new SceneManager(this, id, _ogre_sm, _mesh_manager);
				std::clog << "Renderer : SceneManager created." << std::endl;
			}
			break;

			case OBJ_SCENE_NODE :
			{
				if(!_scene_manager)
				{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("No SceneManager.")); }
				_scene_manager->_createSceneNode(id);
			}
			break;

			case OBJ_MATERIAL_MANAGER :
			{
				if(_material_manager)
				{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Material Manager already created.")); }
				_material_manager.reset(new MaterialManager(this, id));
				std::clog << "Renderer : MaterialManager created" << std::endl;
			}
			break;

			case OBJ_MATERIAL :
			{
				if(!_material_manager)
				{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("NO Material Manager.")); }
				vl::MaterialRefPtr mat = _material_manager->_createMaterial(id);
				_materials_to_check.push_back(mat);
			}
			break;

			/// @todo MovableObjects type should be divided into two one for
			/// the movable object type and other one dynamic so more movable objects
			/// can be created with ease.
			default :
				if(!_scene_manager)
				{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("No SceneManager.")); }
				_scene_manager->_createMovableObject(type, id);
				break;
		}
	}
}

void
vl::Renderer::updateScene(vl::cluster::Message& msg)
{
	assert(msg.getType() == vl::cluster::MSG_SG_UPDATE || msg.getType() == vl::cluster::MSG_SG_INIT);

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


vl::IWindow *
vl::Renderer::createWindow(vl::config::Window const &winConf)
{
	std::cout << vl::TRACE << "vl::Renderer::createWindow : " << winConf.name << std::endl;

	// Destroy dummy window
	if(_windows.size() == 1 && _windows.at(0)->getName() == "dummy")
	{
		delete _windows.at(0);
		_windows.clear();
	}

	vl::Window *window = new vl::Window(winConf, this);
	if(_player)
	{ window->setCamera(_player->getCamera()); }
	_windows.push_back(window);

	return window;
}


/// ------------------------ Protected -----------------------------------------


/// Ogre helpers
void
vl::Renderer::_createOgre(vl::config::EnvSettingsRefPtr env)
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

Ogre::SceneManager *
vl::Renderer::_createOgreSceneManager(vl::ogre::RootRefPtr root, std::string const &name)
{
	assert(root);
	Ogre::SceneManager *sm = _root->createSceneManager(name);

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

					// Check materials, needs to be here because we only have
					// the correct name after first unpack
				_check_materials(iter->getId());
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
	// Update player
	// TODO these should be moved to player using functors
	if( _player )
	{		
		if(!_player->getCamera())
		{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Active Camera name empty.")); }

		for( size_t i = 0; i < _windows.size(); ++i )
		{ _windows.at(i)->setCamera(_player->getCamera()); }

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
vl::Renderer::_takeScreenshot( void )
{
	std::string prefix( "screenshot_" );
	std::string suffix = ".png";

	// Tell the Window to take a screenshot
	for( size_t i = 0; i < _windows.size(); ++i )
	{ _windows.at(i)->takeScreenshot( prefix, suffix ); }
}

void
vl::Renderer::_check_materials(uint64_t const id)
{
	// @todo move to a separate function and maybe
	// make bit more universal (any object can have a callback)
	for(std::vector<vl::MaterialRefPtr>::iterator mat_iter = _materials_to_check.begin();
		mat_iter != _materials_to_check.end(); ++mat_iter)
	{
		if((*mat_iter)->getID() == id)
		{
			assert(_mesh_manager);
			_mesh_manager->checkMaterialUsers(*mat_iter);
			// remove from the check list
			_materials_to_check.erase(mat_iter);
			break;
		}
	}
}
