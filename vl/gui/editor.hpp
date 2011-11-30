/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-09
 *	@file gui/editor.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef HYDRA_GUI_EDITOR_HPP
#define HYDRA_GUI_EDITOR_HPP

#include "gui_window.hpp"

namespace vl
{

namespace gui
{

class EditorWindow : public Window
{
public :
	EditorWindow(vl::gui::GUI *creator);

	~EditorWindow(void);

	/// GECUI callbacks

	/// Menubar buttons clicked
	bool onNewClicked( CEGUI::EventArgs const &e );
	bool onOpenClicked( CEGUI::EventArgs const &e );
	bool onSaveClicked( CEGUI::EventArgs const &e );
	bool onQuitClicked( CEGUI::EventArgs const &e );
	bool onResetClicked( CEGUI::EventArgs const &e );
	bool onImportSceneClicked( CEGUI::EventArgs const &e );
	bool onReloadScenesClicked( CEGUI::EventArgs const &e );
	bool onNewScriptClicked( CEGUI::EventArgs const &e );
	bool onAddScriptClicked( CEGUI::EventArgs const &e );
	bool onReloadScriptsClicked( CEGUI::EventArgs const &e );

	/// Checkboxes
	bool onShowAxisChanged( CEGUI::EventArgs const &e );
	bool onShowNamesChanged( CEGUI::EventArgs const &e );
	bool onShowJointsChanged( CEGUI::EventArgs const &e );

	/// @brief Editor changes
	bool onGraphEditorButtonClicked( CEGUI::EventArgs const &e );
	bool onScriptEditorButtonClicked( CEGUI::EventArgs const &e );
	bool onMaterialEditorButtonClicked( CEGUI::EventArgs const &e );


	// Helper functions
	void addEditor(WindowRefPtr window);

	/// @internal
	void _addEditor(WindowRefPtr window);

// Private virtual overrides
private :

	virtual void doSerialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits) const;

	virtual void doDeserialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits);

	virtual void _window_resetted(void);

// data
private :
	std::vector<WindowRefPtr> _editors;

	std::map<WindowRefPtr, boost::signals::connection> _editors_waiting;

	CEGUI::TabControl *_tab_control;

};	// class EditorWindow


}	// namespace gui

}	// namespace vl

#endif	// HYDRA_GUI_EDITOR_HPP
