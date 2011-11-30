/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-11
 *	@file gui/script_editor.cpp
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
#include "script_editor.hpp"

#include <CEGUI/CEGUI.h>

vl::gui::ScriptEditor::ScriptEditor(vl::gui::GUI *creator)
	: Window(creator, "script_editor.layout")
{
}

vl::gui::ScriptEditor::~ScriptEditor(void)
{
}

void
vl::gui::ScriptEditor::_window_resetted(void)
{
	CEGUI::Combobox *script_list = static_cast<CEGUI::Combobox *>(CEGUI::WindowManager::getSingleton().getWindow("scriptEditor/script_list"));
	if(!script_list)
	{ BOOST_THROW_EXCEPTION(vl::exception()); }

	CEGUI::Image const *sel_img = &CEGUI::ImagesetManager::getSingleton().get("TaharezLook").getImage("ComboboxSelectionBrush");

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
}
