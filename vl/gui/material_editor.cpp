/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-11
 *	@file gui/material_editor.cpp
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
#include "material_editor.hpp"

#include <CEGUI/CEGUI.h>

vl::gui::MaterialEditor::MaterialEditor(vl::gui::GUI *creator)
	: Window(creator, "material_editor.layout")
{
}

vl::gui::MaterialEditor::~MaterialEditor(void)
{
}

void
vl::gui::MaterialEditor::_window_resetted(void)
{
}
