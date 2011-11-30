/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-11
 *	@file gui/scene_graph_editor.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

#include "scene_graph_editor.hpp"

#include <CEGUI/CEGUI.h>

vl::gui::SceneGraphEditor::SceneGraphEditor(vl::gui::GUI *creator)
	: Window(creator, "scene_graph_editor.layout")
{

}

vl::gui::SceneGraphEditor::~SceneGraphEditor(void)
{
}


/// -------------------------- Public CEGUI callbacks ------------------------
bool
vl::gui::SceneGraphEditor::handleTreeEventSelectionChanged(const CEGUI::EventArgs& args)
{
	const CEGUI::TreeEventArgs& treeArgs = static_cast<const CEGUI::TreeEventArgs&>(args);
	CEGUI::Editbox *editBox = (CEGUI::Editbox *)(CEGUI::WindowManager::getSingleton()
		.getWindow("sceneGraphEditor/graph/Editbox"));

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

void
vl::gui::SceneGraphEditor::_window_resetted(void)
{
	/// Some test values for the tree widget
	CEGUI::Tree *node_tree = static_cast<CEGUI::Tree *>(CEGUI::WindowManager::getSingleton().getWindow("sceneGraphEditor/graph/Tree"));
	if(!node_tree)
	{ BOOST_THROW_EXCEPTION(vl::exception()); }

	CEGUI::Image const *sel_img = &CEGUI::ImagesetManager::getSingleton().get("TaharezLook").getImage("ComboboxSelectionBrush");

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
		CEGUI::Event::Subscriber(&vl::gui::SceneGraphEditor::handleTreeEventSelectionChanged, this));
}
