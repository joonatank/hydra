/**	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-09
 *	@file GUI/editor.cpp
 *	
 *	This file is part of Hydra VR game engine.
 */

/// Interface
#include "editor.hpp"

/// Creator
#include "gui.hpp"

#include <CEGUI/CEGUI.h>

#include <boost/bind.hpp>

/// ---------------------------- EditorWindow -------------------------------
/// ------------------------------ Public -----------------------------------
vl::gui::EditorWindow::EditorWindow(vl::gui::GUI *creator)
	: Window(creator, "editor.layout")
	, _script_editor(0)
	, _material_editor(0)
	, _graph_editor(0)
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

bool
vl::gui::EditorWindow::handleTreeEventSelectionChanged(const CEGUI::EventArgs& args)
{
	const CEGUI::TreeEventArgs& treeArgs = static_cast<const CEGUI::TreeEventArgs&>(args);
	CEGUI::Editbox *editBox = (CEGUI::Editbox *)(CEGUI::WindowManager::getSingleton().getWindow("sceneGraphEditor/graph/Editbox"));

// Three different ways to get the item selected.
//   TreeCtrlEntry *selectedItem = theTree->getFirstSelectedItem();      // the first selection in the list (may be more)
//   TreeCtrlEntry *selectedItem = theTree->getLastSelectedItem();       // the last (time-wise) selected by the user
	CEGUI::TreeItem *selectedItem = treeArgs.treeItem;                    // the actual item that caused this event

	if (selectedItem)
	{ editBox->setText("Selected: " + selectedItem->getText()); }
	else
	{ editBox->setText("None Selected"); }

	return true;
}

/// ------------------------------ Private -----------------------------------
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


    CEGUI::TabControl *tc = static_cast<CEGUI::TabControl *>(_window->getChildRecursive("editor/Frame/editor_tabs"));
	assert(tc);

    // Add some pages to tab control
    tc->addTab(CEGUI::WindowManager::getSingleton().loadWindowLayout("scene_graph_editor.layout"));
    tc->addTab(CEGUI::WindowManager::getSingleton().loadWindowLayout("material_editor.layout"));
	tc->addTab(CEGUI::WindowManager::getSingleton().loadWindowLayout("script_editor.layout"));
	tc->addTab(CEGUI::WindowManager::getSingleton().loadWindowLayout("configuration.layout"));

	CEGUI::Combobox *script_list = static_cast<CEGUI::Combobox *>(CEGUI::WindowManager::getSingleton().getWindow("scriptEditor/script_list"));
	const CEGUI::Image* sel_img = &CEGUI::ImagesetManager::getSingleton().get("TaharezLook").getImage("ComboboxSelectionBrush");

	/// Some test values for the script list
	CEGUI::ListboxItem *list_item = new CEGUI::ListboxTextItem("script.py", 0);
	list_item->setSelectionBrushImage(sel_img);
	script_list->addItem(list_item);
	list_item = new CEGUI::ListboxTextItem("physics_script.py", 1);
	list_item->setSelectionBrushImage(sel_img);
	script_list->addItem(list_item);
	list_item = new CEGUI::ListboxTextItem("meh.py", 2);
	list_item->setSelectionBrushImage(sel_img);
	script_list->addItem(list_item);

	/// Some test values for the tree widget
	CEGUI::Tree *node_tree = static_cast<CEGUI::Tree *>(CEGUI::WindowManager::getSingleton().getWindow("sceneGraphEditor/graph/Tree"));
	CEGUI::TreeItem *tree_item = new CEGUI::TreeItem("test");
	tree_item->setSelectionBrushImage(sel_img);
	node_tree->addItem(tree_item);
	tree_item = new CEGUI::TreeItem("node1");
	tree_item->setSelectionBrushImage(sel_img);
	node_tree->addItem(tree_item);
	tree_item = new CEGUI::TreeItem("node2");
	tree_item->setSelectionBrushImage(sel_img);
	node_tree->addItem(tree_item);
	tree_item = new CEGUI::TreeItem("node3");
	tree_item->setSelectionBrushImage(sel_img);
	node_tree->addItem(tree_item);

	node_tree->subscribeEvent(CEGUI::Tree::EventSelectionChanged, 
		CEGUI::Event::Subscriber(&vl::gui::EditorWindow::handleTreeEventSelectionChanged, this));
}
