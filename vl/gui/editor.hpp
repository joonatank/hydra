/**	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-09
 *	@file GUI/editor.hpp
 *	
 *	This file is part of Hydra VR game engine.
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

	bool handleTreeEventSelectionChanged(const CEGUI::EventArgs& args);

// Private virtual overrides
private :
	virtual void _window_resetted(void);

// data
private :
	CEGUI::Window *_script_editor;
	CEGUI::Window *_material_editor;
	CEGUI::Window *_graph_editor;

};	// class EditorWindow


}	// namespace gui

}	// namespace vl

#endif	// HYDRA_GUI_EDITOR_HPP
