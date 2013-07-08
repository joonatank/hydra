/**
 *	Copyright (c) 2011 - 2013 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2013-07
 *	@file pipe.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

#include "pipe.hpp"

#include <iostream>

#include "window.hpp"
// Necessary for adding resource paths
#include "settings.hpp"
#include "renderer.hpp"

/// Master constructor
vl::Pipe::Pipe(vl::Session *session, std::string const &name)
	: _session(session) 
	, _name(name)
	, _gui_enabled(false)
	, _debug_overlay_enabled(false)
{
	assert(session);
	_session->registerObject(this, OBJ_PIPE);
}
	
/// Slave constructor
vl::Pipe::Pipe(vl::Session *session, uint64_t id)
	: _session(session)
	, _renderer(0)
	, _gui_enabled(false)
	, _debug_overlay_enabled(false)
{
	assert(_session);
	_session->registerObject(this, id);
}

vl::Pipe::~Pipe(void)
{
	std::clog << "vl::Pipe::~Pipe" << std::endl;

	/* @todo can we delete these here?
	 * should they be deregistered first etc.
	std::vector<Window *>::iterator iter;
	for( iter = _windows.begin(); iter != _windows.end(); ++iter )
	{ delete *iter; }
	_windows.clear();
	 */
}

void
vl::Pipe::capture(void)
{
	for( size_t i = 0; i < getWindows().size(); ++i )
	{ getWindows().at(i)->capture(); }
}
	
void
vl::Pipe::draw(void)
{
	for( size_t i = 0; i < getWindows().size(); ++i )
	{ getWindows().at(i)->draw(); }
}
	
void
vl::Pipe::swap(void)
{
	for( size_t i = 0; i < getWindows().size(); ++i )
	{ getWindows().at(i)->swap(); }
}


void
vl::Pipe::enableDebugOverlay(bool enable)
{
	if(enable != _debug_overlay_enabled)
	{
		setDirty(DIRTY_PARAMS);
		_debug_overlay_enabled = enable;
	}
}

void
vl::Pipe::enableGUI(bool enable)
{
	if(enable != _gui_enabled)
	{
		setDirty(DIRTY_PARAMS);
		_gui_enabled = enable;
	}
}

void
vl::Pipe::addResources(vl::Settings const &settings)
{
	setDirty(DIRTY_RESOURCES);

	// Add resources
	if(!settings.getProjectDir().empty())
	{ _resource_paths.push_back(settings.getProjectDir()); }
	for(size_t i = 0; i < settings.getAuxDirectories().size(); ++i)
	{
		if(!settings.getAuxDirectories().at(i).empty())
		{ _resource_paths.push_back(settings.getAuxDirectories().at(i)); }
	}
}

void
vl::Pipe::clearResources(void)
{
	setDirty(DIRTY_RESOURCES);

	_resource_paths.clear();
}

vl::IWindow *
vl::Pipe::createWindow(vl::config::Window const &winConf)
{
	std::clog << "vl::Pipe::createWindow : " << winConf.name << std::endl;

	setDirty(DIRTY_WINDOWS);
	
	vl::Window *window = new vl::Window(winConf, this);
	_windows.push_back(window);
	/*	@todo Player is not accessable here
	if(_player)
	{ window->setCamera(_player->getCamera()); }
	_windows.push_back(window);
	*/

	return window;
}

void
vl::Pipe::_windowCreated(Window *win)
{
	if(_hasWindow(win->getID()))
	{ return; }

	std::vector<uint64_t>::iterator iter;
	for(iter = _window_ids.begin();
		iter != _window_ids.end(); ++iter)
	{
		if(*iter == win->getID())
		{
			std::clog << "vl::Pipe::_windowCreated : Window added." << std::endl;
			_windows.push_back(win);
			break;
		}
	}
	// Remove the ids from the vector when we have the actual window
	if(iter != _window_ids.end())
	{
		_window_ids.erase(iter);
	}
}

void
vl::Pipe::serialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits ) const
{
	if( DIRTY_NAME & dirtyBits )
	{
		msg << _name;
	}

	if( DIRTY_WINDOWS & dirtyBits )
	{
		std::clog << "Serializing : New windows created in Pipe." << std::endl;
		msg << _windows.size();
		for(size_t i = 0; i < _windows.size(); ++i)
		{
			msg << _windows.at(i)->getID();
		}
	}

	if( DIRTY_PARAMS & dirtyBits )
	{
		msg << _gui_enabled << _debug_overlay_enabled;
	}

	if( DIRTY_RESOURCES & dirtyBits )
	{
		msg << _resource_paths;
	}
}

void
vl::Pipe::deserialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits )
{
	if( DIRTY_NAME & dirtyBits )
	{
		msg >> _name;
	}

	if( DIRTY_WINDOWS & dirtyBits )
	{
		std::clog << "Deserializing : New windows created in Pipe." << std::endl;
		size_t size;
		msg >> size;
		for(size_t i = 0; i < size; ++i)
		{
			uint64_t id;
			msg >> id;
			// Just testing if we can retrieve Windows here
			// we can so we should cast it and save it
			vl::Distributed *obj = _session->findMappedObject(id);
			if(obj)
			{
				std::clog << "Found window with id " << id << std::endl;
				if(!_hasWindow(id))
				{
					std::clog << "Adding Window to Slave Pipe" << std::endl;
					_windows.push_back((Window *)obj);
				}
			}
			else
			{
				// @todo this is bad
				// this fails so we need another way to add Windows
				_window_ids.push_back(id);
			}
		}
	}

	if( DIRTY_PARAMS & dirtyBits )
	{
		msg >> _gui_enabled >> _debug_overlay_enabled;
	}

	if( DIRTY_RESOURCES & dirtyBits )
	{
		msg >> _resource_paths;

		// Reset ogre resources if already added
		if(_renderer)
		{ _renderer->setResources(_resource_paths); }
	}
}

void
vl::Pipe::setRenderer(vl::Renderer *renderer)
{
	assert(!_renderer && renderer);
	_renderer = renderer;

	// Copy variables
	_renderer->setResources(_resource_paths);
}


bool
vl::Pipe::_hasWindow(uint64_t id)
{
	return( _findWindow(id) != 0 );
}

vl::Window *
vl::Pipe::_findWindow(uint64_t id)
{
	for(size_t i = 0; i < _windows.size(); ++i)
	{
		if(_windows.at(i)->getID() == id)
		{ return _windows.at(i); }
	}

	return 0;
}
