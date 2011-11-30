/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-11
 *	@file gui/script_editor.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef HYDRA_GUI_SCRIPT_EDITOR_HPP
#define HYDRA_GUI_SCRIPT_EDITOR_HPP

#include "gui_window.hpp"

namespace vl
{

namespace gui
{

/// @todo stub for now
/// needs bit more elaborate structure to work with CEGUI
/// mainly either lots of getNative calls or thin wrappers
class ScriptEditor : public gui::Window
{
public :
	ScriptEditor(vl::gui::GUI *creator);

	virtual ~ScriptEditor(void);

/// Private virtual overrides
private :
	void _window_resetted(void);
};

}	// namespace gui

}	// namespace vl

#endif	// HYDRA_GUI_SCRIPT_EDITOR_HPP
