/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-09
 *	@file gui/editor.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

/// Interface
#include "editor.hpp"

/// Creator
#include "gui.hpp"

#include <CEGUI/CEGUI.h>

/// ---------------------------- EditorWindow -------------------------------
/// ------------------------------ Public -----------------------------------
vl::gui::EditorWindow::EditorWindow(vl::gui::GUI *creator)
	: Window(creator, "editor.layout")
	, _tab_control(0)
{
}

vl::gui::EditorWindow::~EditorWindow(void)
{
}

/// ------------------------ Public CEGUI callbacks ----------------------------
bool
vl::gui::EditorWindow::onNewClicked( CEGUI::EventArgs const &e )
{
	std::cout << vl::TRACE << "vl::Window::onNewClicked" << std::endl;

	return true;
}

bool
vl::gui::EditorWindow::onOpenClicked( CEGUI::EventArgs const &e )
{
	std::cout << vl::TRACE << "vl::Window::onOpenClicked" << std::endl;

	return true;
}

bool
vl::gui::EditorWindow::onSaveClicked( CEGUI::EventArgs const &e )
{
	std::cout << vl::TRACE << "vl::Window::onSaveClicked" << std::endl;

	return true;
}

bool
vl::gui::EditorWindow::onQuitClicked( CEGUI::EventArgs const &e )
{
	std::cout << vl::TRACE << "vl::Window::onQuitClicked" << std::endl;

	_creator->sendCommand( "quit()" );

	return true;
}

bool
vl::gui::EditorWindow::onResetClicked( CEGUI::EventArgs const &e )
{
	std::cout << vl::TRACE << "vl::Window::onResetClicked" << std::endl;

	return true;
}

bool
vl::gui::EditorWindow::onImportSceneClicked( CEGUI::EventArgs const &e )
{
	std::cout << vl::TRACE << "vl::Window::onImportSceneClicked" << std::endl;

	return true;
}

bool
vl::gui::EditorWindow::onReloadScenesClicked( CEGUI::EventArgs const &e )
{
	std::cout << vl::TRACE << "vl::Window::onReloadScenesClicked" << std::endl;

	return true;
}


bool
vl::gui::EditorWindow::onNewScriptClicked( CEGUI::EventArgs const &e )
{
	std::cout << vl::TRACE << "vl::Window::onNewScriptClicked" << std::endl;

	return true;
}

bool
vl::gui::EditorWindow::onAddScriptClicked( CEGUI::EventArgs const &e )
{
	std::cout << vl::TRACE << "vl::Window::onAddScriptClicked" << std::endl;

	return true;
}

bool
vl::gui::EditorWindow::onReloadScriptsClicked( CEGUI::EventArgs const &e )
{
	std::cout << vl::TRACE << "vl::Window::onReloadScriptsClicked" << std::endl;

	return true;
}

bool
vl::gui::EditorWindow::onShowAxisChanged( CEGUI::EventArgs const &e )
{
	std::cout << vl::TRACE << "vl::Window::onShowAxisChanged" << std::endl;

	return true;
}

bool
vl::gui::EditorWindow::onShowNamesChanged( CEGUI::EventArgs const &e )
{
	std::cout << vl::TRACE << "vl::Window::onShowNamesChanged" << std::endl;

	return true;
}

bool
vl::gui::EditorWindow::onShowJointsChanged( CEGUI::EventArgs const &e )
{
	std::cout << vl::TRACE << "vl::Window::onShowJointsChanged" << std::endl;

	return true;
}

bool
vl::gui::EditorWindow::onGraphEditorButtonClicked( CEGUI::EventArgs const &e )
{
	std::cout << vl::TRACE << "vl::Window::onGraphEditorButtonClicked" << std::endl;

	return true;
}

bool
vl::gui::EditorWindow::onScriptEditorButtonClicked( CEGUI::EventArgs const &e )
{
	std::cout << vl::TRACE << "vl::Window::onScriptEditorButtonClicked" << std::endl;

	return true;
}

bool
vl::gui::EditorWindow::onMaterialEditorButtonClicked( CEGUI::EventArgs const &e )
{
	std::cout << vl::TRACE << "vl::Window::onMaterialEditorButtonClicked" << std::endl;

	return true;
}

void
vl::gui::EditorWindow::addEditor(WindowRefPtr window)
{
	_editors.push_back(window);

	if(_tab_control)
	{
		assert(window->getNative());
		_tab_control->addTab(window->getNative());
	}
}

void
vl::gui::EditorWindow::_addEditor(vl::gui::WindowRefPtr window)
{
	assert(window->getNative());
	_tab_control->addTab(window->getNative());
	
	// Unsubcribe and remove from connection list
	std::map<WindowRefPtr, boost::signals::connection>::iterator iter = _editors_waiting.find(window);
	assert(iter != _editors_waiting.end());
	if(iter != _editors_waiting.end())
	{
		window->removeListener(iter->second);
	}
	_editors_waiting.erase(iter);
}

/// ------------------------------ Private -----------------------------------

void
vl::gui::EditorWindow::doSerialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits) const
{
}

void
vl::gui::EditorWindow::doDeserialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits)
{
}

void
vl::gui::EditorWindow::_window_resetted(void)
{
	assert(_window);
	// @todo should check that we have a correct type of a window

	/* Moved to separate menu window
	CEGUI::MenuItem *item = static_cast<CEGUI::MenuItem *>( _window->getChildRecursive("menu/new") );
	assert( item );
	item->subscribeEvent(CEGUI::MenuItem::EventClicked, CEGUI::Event::Subscriber(&vl::gui::EditorWindow::onNewClicked, this));

	item = static_cast<CEGUI::MenuItem *>( _window->getChildRecursive("menu/open") );
	assert( item );
	item->subscribeEvent(CEGUI::MenuItem::EventClicked, CEGUI::Event::Subscriber(&vl::gui::EditorWindow::onOpenClicked, this));

	item = static_cast<CEGUI::MenuItem *>( _window->getChildRecursive("menu/save") );
	assert( item );
	item->subscribeEvent(CEGUI::MenuItem::EventClicked, CEGUI::Event::Subscriber(&vl::gui::EditorWindow::onSaveClicked, this));

	item = static_cast<CEGUI::MenuItem *>( _window->getChildRecursive("menu/quit") );
	assert( item );
	item->subscribeEvent(CEGUI::MenuItem::EventClicked, CEGUI::Event::Subscriber(&vl::gui::EditorWindow::onQuitClicked, this));

	item = static_cast<CEGUI::MenuItem *>( _window->getChildRecursive("menu/reset") );
	assert( item );
	item->subscribeEvent(CEGUI::MenuItem::EventClicked, CEGUI::Event::Subscriber(&vl::gui::EditorWindow::onResetClicked, this));
	*/

	/* Moved to scene graph editor
	item = static_cast<CEGUI::MenuItem *>( _window->getChildRecursive("editor/importSceneItem") );
	assert( item );
	item->subscribeEvent(CEGUI::MenuItem::EventClicked, CEGUI::Event::Subscriber(&vl::gui::EditorWindow::onImportSceneClicked, this));

	item = static_cast<CEGUI::MenuItem *>( _window->getChildRecursive("editor/reloadScenes") );
	assert( item );
	item->subscribeEvent(CEGUI::MenuItem::EventClicked, CEGUI::Event::Subscriber(&vl::gui::EditorWindow::onReloadScenesClicked, this));
	*/

	/* Moved to script editor
	item = static_cast<CEGUI::MenuItem *>( _window->getChildRecursive("editor/addScriptItem") );
	assert( item );
	item->subscribeEvent(CEGUI::MenuItem::EventClicked, CEGUI::Event::Subscriber(&vl::gui::EditorWindow::onAddScriptClicked, this));

	item = static_cast<CEGUI::MenuItem *>( _window->getChildRecursive("editor/newScriptItem") );
	assert( item );
	item->subscribeEvent(CEGUI::MenuItem::EventClicked, CEGUI::Event::Subscriber(&vl::gui::EditorWindow::onNewScriptClicked, this));

	item = static_cast<CEGUI::MenuItem *>( _window->getChildRecursive("editor/reloadScripts") );
	assert( item );
	item->subscribeEvent(CEGUI::MenuItem::EventClicked, CEGUI::Event::Subscriber(&vl::gui::EditorWindow::onReloadScriptsClicked, this));
	*/

	/* Moved to configuration
	CEGUI::Checkbox *checkBox = static_cast<CEGUI::Checkbox *>( _window->getChildRecursive("editor/showAxes") );
	assert( checkBox );
	checkBox->subscribeEvent(CEGUI::Checkbox::EventCheckStateChanged, CEGUI::Event::Subscriber(&vl::gui::EditorWindow::onShowAxisChanged, this));

	checkBox = static_cast<CEGUI::Checkbox *>( _window->getChildRecursive("editor/showNames") );
	assert( checkBox );
	checkBox->subscribeEvent(CEGUI::Checkbox::EventCheckStateChanged, CEGUI::Event::Subscriber(&vl::gui::EditorWindow::onShowNamesChanged, this));

	checkBox = static_cast<CEGUI::Checkbox *>( _window->getChildRecursive("editor/showJoints") );
	assert( checkBox );
	checkBox->subscribeEvent(CEGUI::Checkbox::EventCheckStateChanged, CEGUI::Event::Subscriber(&vl::gui::EditorWindow::onShowJointsChanged, this));
	*/

	/* Removed
	CEGUI::PushButton *button = static_cast<CEGUI::PushButton *>( _window->getChildRecursive("editor/graphEditorButton") );
	assert(button);
	button->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&vl::gui::EditorWindow::onGraphEditorButtonClicked, this));

	button = static_cast<CEGUI::PushButton *>( _window->getChildRecursive("editor/scriptEditorButton") );
	assert(button);
	button->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&vl::gui::EditorWindow::onScriptEditorButtonClicked, this));

	button = static_cast<CEGUI::PushButton *>( _window->getChildRecursive("editor/materialEditorButton") );
	assert(button);
	button->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&vl::gui::EditorWindow::onMaterialEditorButtonClicked, this));
	*/

	/*
	_script_editor = static_cast<CEGUI::PushButton *>( _window->getChildRecursive("editor/scriptEditor") );
	_material_editor = static_cast<CEGUI::PushButton *>( _window->getChildRecursive("editor/materialEditor") );
	_graph_editor = static_cast<CEGUI::PushButton *>( _window->getChildRecursive("editor/graphEditor") );
	assert(_script_editor && _material_editor && _graph_editor);
	_script_editor->hide();
	_material_editor->hide();
	_graph_editor->hide();
	*/

	_tab_control = static_cast<CEGUI::TabControl *>(_window->getChildRecursive("editor/Frame/editor_tabs"));
	assert(_tab_control);

	for(std::vector<WindowRefPtr>::iterator iter = _editors.begin(); 
		iter != _editors.end(); ++iter)
	{
		if(!(*iter)->getNative())
		{
			assert(_editors_waiting.find(*iter) == _editors_waiting.end());
			// Some really nifty things you can do with boost::bind
			// calls this->_addEditor(*iter) when the signal is evoked
			_editors_waiting[*iter] = (*iter)->addListener(boost::bind(boost::bind(&EditorWindow::_addEditor, this, _1), *iter));
		}
		_tab_control->addTab((*iter)->getNative());
	}

	/// @todo move to separate window class
	_tab_control->addTab(CEGUI::WindowManager::getSingleton().loadWindowLayout("configuration.layout"));
}
