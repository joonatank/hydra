/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2013 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-01
 *	@file renderer.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */


// Interface
#include "renderer.hpp"

// Necessary for printing error messages from exceptions
#include "base/exceptions.hpp"

#include "window.hpp"
#include "base/string_utils.hpp"
#include "base/sleep.hpp"
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
#include "mesh_manager.hpp"

#include "pipe.hpp"

// Necessary for message pump
#include <OGRE/OgreWindowEventUtilities.h>

#include <boost/bind.hpp>

/// ------------------------- Public -------------------------------------------
vl::Renderer::Renderer(vl::Session *session, std::string const &name)
	: _name(name)
	, _session(session)
	, _pipe(0)
	, _scene_manager(0)
	, _player(0)
	, _screenshot_num(0)
	, _n_log_messages(0)
{
	std::cout << vl::TRACE << "vl::Renderer::Renderer : name = " << _name << std::endl;
}

vl::Renderer::~Renderer(void)
{
	std::cout << vl::TRACE << "vl::Renderer::~Renderer" << std::endl;

	// Necessary because this holds Ogre resources.
	_material_manager.reset();

	_root.reset();

	delete _scene_manager;
	delete _player;

	// Don't delete pipe since it's a pointer in _all_pipes
	_pipe = 0;
	for(size_t i = 0; i < _all_pipes.size(); ++i)
	{ delete _all_pipes.at(i); }

	std::cout << vl::TRACE << "vl::Renderer::~Renderer : DONE" << std::endl;
}

void
vl::Renderer::init(void)
{
	std::cout << vl::TRACE << "vl::Renderer::init" << std::endl;

	// Don't necessarily need the log level thingy
	// Release version has horrible amount of empty lines if we use LL_NORMAL
	// Debug version is fine
#ifdef _DEBUG
	_root.reset( new vl::ogre::Root(config::LL_NORMAL) );
#else
	_root.reset( new vl::ogre::Root(config::LL_LOW) );
#endif
	// Initialise ogre
	_root->createRenderSystem();
}

/// --------------------------------------------------------------------------
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

/// --------------------------------------------------------------------------
/// ----------------------- Rendering ----------------------------------------
void
vl::Renderer::capture(void)
{
	if(!_pipe)
	{ return; }

	// Process input events
	_pipe->capture();
}

void
vl::Renderer::draw(void)
{
	if(!_pipe)
	{ return; }

	Ogre::WindowEventUtilities::messagePump();

	// @todo move the callback notifiying to Root
	_root->getNative()->_fireFrameStarted();

	if(_scene_manager)
	{ _scene_manager->_notifyFrameStart(); }

	if(_pipe)
	{ _pipe->draw(); }

	// Copy view and projection matrices
	// @todo should be in the pipe or something
	if(_pipe->getWindows().size() > 0 && _pipe->getWindows().at(0)->getChannels().size() > 0)
	{
		// @todo does this work for deferred shading or FBOs?
		// what projection and view matrices exacatly are left after
		// rendering a frame?
		// need to test more but view matrix seemed fine for deferred.
		Channel *chan = _pipe->getWindows().at(0)->getChannels().at(0);

		vl::Transform t = chan->getCamera().getViewTransform();
		_view_matrix = Ogre::Matrix4(t.quaternion);
		_view_matrix.setTrans(t.position);
		_proj_matrix = chan->getCamera().getFrustum().getProjectionMatrix();
	}

	// Hack to update Sky
	if(_pipe->getWindows().size() > 0 && _player->getCamera()
		&& _scene_manager && _scene_manager->getSkySimulator())
	{
		_scene_manager->getSkySimulator()->notifyCameraRender(_player->getCamera());
	}

	if(_scene_manager)
	{ _scene_manager->_notifyFrameEnd(); }

	_root->getNative()->_fireFrameEnded();
}

void
vl::Renderer::swap(void)
{
	if(!_pipe)
	{ return; }

	_pipe->swap();
}

/// --------------------------------------------------------------------------
/// ----------------------- Project management -------------------------------
void
vl::Renderer::setResources(std::vector<std::string> const &paths)
{
	assert(_root);
	assert(_pipe);
	
	// remove all old resources
	// Needs to be here because we need the global resources to be available 
	// all times. When project is unloaded we leave old resources so that
	// shadow materials and Gorilla resources are available till new project
	// is loaded.
	_root->removeResources();

	std::clog << "Setting up the Ogre resources." << std::endl;

	// add all resources
	_root->setupResources(paths);
}

void
vl::Renderer::clearProject(void)
{
	std::clog << "vl::Renderer::clearProject" << std::endl;

	if(_scene_manager)
	{
		_scene_manager->destroyScene();
	}

	// Remove cameras and wait for them to be reset
	if(_pipe)
	{
		for(size_t i = 0; i < _pipe->getWindows().size(); ++i)
		{
			_pipe->getWindows().at(i)->setCamera(0);
			_pipe->getWindows().at(i)->resetStatistics();
		}
	}
}

/// --------------------------------------------------------------------------
void
vl::Renderer::initialiseGUI(vl::ChannelPtr channel)
{
	assert(_gui);

	// NOP if already initialised
	if(_gui->initialised() || _gui->getChannel())
	{ return; }

	std::clog << "vl::Renderer::_initialiseGUI" << std::endl;

	assert(channel);
	
	_gui->setChannel(channel);
}

/// ----------------------- Syncing -----------------------------------------
void
vl::Renderer::createSceneObjects(vl::cluster::Message& msg)
{
	std::clog << "vl::Renderer::createSceneObjects" << std::endl;

	assert(msg.getType() == vl::cluster::MSG_SG_CREATE);
	
	std::string err_no_data("Message does not have enough bytes.");
	size_t size;
	if( msg.size() < sizeof(size) )
	{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(err_no_data)); }

	msg.read( size );
	for( size_t i = 0; i < size; ++i )
	{
		if( msg.size() < (sizeof(OBJ_TYPE) + sizeof(uint64_t)) )
		{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(err_no_data)); }
		
		OBJ_TYPE type;
		uint64_t id;

		msg.read(type);
		msg.read(id);

		assert(_objects_not_yet_created.find(id) == _objects_not_yet_created.end());
		// @todo this should be ordered set based on type not ID
		// as long as types are ordered based on creation order
		// (as long as first objects have lower id they are created first)
		// we need to add a new container for this though.
		_objects_not_yet_created[id] = type;
	}
}

void
vl::Renderer::_create_objects(IdTypeMap const &objects, IdTypeMap &left_overs)
{
	for(std::map<uint64_t, OBJ_TYPE>::const_iterator iter = objects.begin();
		iter != objects.end(); ++iter)
	{
		uint64_t id = iter->first;
		switch(iter->second)
		{
			case OBJ_PIPE :
			{
				std::clog << "Creating Pipe with id " << id << std::endl;
				_all_pipes.push_back(new Pipe(_session, id));
			}
			break;

			case OBJ_WINDOW :
			{
				if(_pipe)
				{
					std::clog << "Creating a Window with id " << id << std::endl;
					Window *win = new Window(this, id);
					
					// Add the window to the Pipe
					// this can't be done automatically by the Pipe because it's
					// deserialised first (we need the proper pipe for this Renderer)
					_pipe->_windowCreated(win);
				}
				else
				{ left_overs[id] = iter->second; }
			}
			break;

			case OBJ_CHANNEL :
			{
				std::clog << "Channel objects NOT YET SUPPORTED." << std::endl;
			}
			break;

			case OBJ_PLAYER :
			{
				// Create the Player if we have a SceneManager
				if(_scene_manager)
				{
					std::clog << "Creating Player with ID : " << id << std::endl;
					vl::Player *player = new vl::Player(_scene_manager);
					// @todo this needs to use Slave
					_session->registerObject(player, id);
				
					// Only single instances are supported for now
					assert(!_player && player);
					_player = player;
				}
				// Store it for later if we don't
				else
				{ left_overs[id] = iter->second; }
			}
			break;

			case OBJ_GUI :
			{
				if(_pipe)
				{
					std::clog << "Renderer : Creating GUI with id " << id << std::endl;

					// Do not create the GUI multiple times
					if(_gui)
					{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("GUI already created")); }

					_gui.reset(new vl::gui::GUI(_session, id));
				}
				// store for later
				else
				{ left_overs[id] = iter->second; }
			}
			break;

			case OBJ_GUI_CONSOLE :
			case OBJ_GUI_PERFORMANCE_OVERLAY:
			{
				if(_pipe && _gui->initialised())
				{
					std::clog << "Renderer : Creating GUI Widnow" << std::endl;
					if(!_gui)
					{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("NO GUI when trying to create GUI::Window.")); }
					_gui->createWindow(iter->second, id);
					if(iter->second == OBJ_GUI_CONSOLE)
					{
						_gui->getConsole()->addCommandListener(boost::bind(&Renderer::sendCommand, this, _1));
					}
				}
				// store for later
				else
				{ left_overs[id] = iter->second; }
			}
			break;

			case OBJ_SCENE_MANAGER :
			{
				if(_pipe)
				{
					std::clog << "Renderer : Creating SceneManager" << std::endl;
					if(_scene_manager)
					{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("SceneManager already created.")); }
					if(!_mesh_manager)
					{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("No MeshManager.")); }

					Ogre::SceneManager *sm = _root->createSceneManager("SceneManager");
					_scene_manager = new SceneManager(_session, id, sm, _mesh_manager);
				}
				// store for later
				else
				{ left_overs[id] = iter->second; }
			}
			break;

			case OBJ_SCENE_NODE :
			{
				if(_scene_manager)
				{ _scene_manager->_createSceneNode(id); }
				else
				{ left_overs[id] = iter->second; }
			}
			break;

			case OBJ_MATERIAL_MANAGER :
			{
				std::clog << "Renderer : Creating MaterialManager" << std::endl;
				if(_material_manager)
				{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Material Manager already created.")); }
				_material_manager.reset(new MaterialManager(_session, id));
				std::clog << "Renderer : MaterialManager created" << std::endl;
			}
			break;

			case OBJ_MATERIAL :
			{
				if(_material_manager)
				{
					vl::MaterialRefPtr mat = _material_manager->_createMaterial(id);
					_materials_to_check.push_back(mat);
				}
				else
				{ left_overs[id] = iter->second; }
			}
			break;

			/// @todo MovableObjects type should be divided into two one for
			/// the movable object type and other one dynamic so more movable objects
			/// can be created with ease.
			default :
			{
				// Movable object
				if(iter->second >= OBJ_MOVABLE)
				{
					if(_scene_manager)
					{ _scene_manager->_createMovableObject(iter->second, id); }
					else
					{ left_overs[id] = iter->second; }
				}
				else
				{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Unknown distributed object type.")); }
			}
			break;
		}
	}
}

void
vl::Renderer::updateScene(vl::cluster::Message& msg)
{
	assert(msg.getType() == vl::cluster::MSG_SG_UPDATE || msg.getType() == vl::cluster::MSG_SG_INIT);

	// This method works whenever we have a create message
	// divided into two separate messages in the same or concecutive
	// frames.
	// Because first message is going to leave some objects not created
	// second causes us to re run this function and create all of them.
	// This depends on the ordering of the objects so manager objects
	// should have lowest ids which the current implementation enforces.
	// because ids are ordered by creation and managers need to be created
	// first.
	// We do the checking here instead of createSceneObjects() because
	// we added some objects that require ordered creation.
	//
	// @todo might move to separate function that iterates over the array
	// till no more objects can be created this frame.
	if(_objects_not_yet_created.size() > 0)
	{
		IdTypeMap saved_for_later;
		_create_objects(_objects_not_yet_created, saved_for_later);

		size_t n_objects = _objects_not_yet_created.size();
		size_t n_saved = saved_for_later.size();
		size_t n_created = n_objects - n_saved;
		if(n_objects > 0)
		{
			std::clog << n_created << " objects created from " << n_objects
				<< " and " << n_saved << " objects were left to be created later." << std::endl;
		}

		_objects_not_yet_created = saved_for_later;
	}

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

/// --------------------------------------------------------------------------
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
			vl::Distributed *obj = _session->findMappedObject( iter->getId() );
			if( obj )
			{
				obj->unpack(stream);

				// Check materials, needs to be here because we only have
				// the correct name after first unpack
				_check_materials(iter->getId());
			}
			else
			{
				std::clog << "No ID " << iter->getId() 
					<< " found in mapped objects. Saving for later."
					<< std::endl;
				tmp.push_back( *iter );
			}
		}
	}

	_objects = tmp;

	/// Find our Pipe
	if(!_pipe)
	{
		std::clog << "Trying to find Pipe for : " << _name 
			<< " we have " << _all_pipes.size() << " pipes available." << std::endl;
		for(size_t i = 0; i < _all_pipes.size(); ++i)
		{
			if(_all_pipes.at(i)->getName() == _name)
			{
				std::clog << "Renderer Found a Pipe" << std::endl;
				_pipe = _all_pipes.at(i);
				_pipe->setRenderer(this);
				break;
			}
		}	
	}
}

void
vl::Renderer::_updateDistribData( void )
{
	// Update player
	// TODO these should be moved to player using functors
	if( _player && _pipe)
	{		
		if(!_player->getCamera())
		{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Active Camera name empty.")); }
		
		for( size_t i = 0; i < _pipe->getWindows().size(); ++i )
		{ _pipe->getWindows().at(i)->setCamera(_player->getCamera()); }

		// Take a screenshot
		if( _player->getScreenshotVersion() > _screenshot_num )
		{
			_takeScreenshot();

			_screenshot_num = _player->getScreenshotVersion();
		}
	}
}

/// @todo move this to public interface
void
vl::Renderer::_takeScreenshot( void )
{
	if(!_pipe)
	{ return; }

	std::string prefix( "screenshot_" );
	std::string suffix = ".png";

	// Tell the Window to take a screenshot
	for( size_t i = 0; i < _pipe->getWindows().size(); ++i )
	{ _pipe->getWindows().at(i)->takeScreenshot( prefix, suffix ); }
}

/// @todo document the purpose of this and clean it up a little bit
/// mostly the problem is when is this supposed to be used?
/// what are the start and end states
void
vl::Renderer::_check_materials(uint64_t const id)
{
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
