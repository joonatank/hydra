/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-02
 *	@file gui.hpp
 *
 */

#ifndef VL_GUI_GUI_HPP
#define VL_GUI_GUI_HPP

#include "distributed.hpp"
#include "session.hpp"
#include "action.hpp"

#include "window.hpp"

namespace vl
{

namespace gui
{

/**	@class GUI
 *	@brief Distributed class for GUI modifications
 */
class GUI : public vl::Distributed
{
public :
	GUI( vl::Session *session, uint64_t id = vl::ID_UNDEFINED )
		: _editor_shown(false)
		, _console_shown(false)
		, _stats_shown(false)
		, _loading_screen_shown(false)
	{
		session->registerObject(this, OBJ_GUI, id);
	}

	void setEditor( WindowRefPtr win )
	{ _editor = win; }

	void showEditor( void )
	{
		if( !_editor_shown )
		{
			setDirty(DIRTY_EDITOR);
			_editor_shown = true;
		}
	}

	void hideEditor( void )
	{
		if( _editor_shown )
		{
			setDirty(DIRTY_EDITOR);
			_editor_shown = false;
		}
	}

	bool editorShown( void ) const
	{ return _editor_shown; }

	void setConsole( WindowRefPtr win )
	{ _console = win; }

	void showConsole( void )
	{
		if( !_console_shown )
		{
			setDirty(DIRTY_CONSOLE);
			_console_shown = true;
		}
	}

	void hideConsole( void )
	{
		if( _console_shown )
		{
			setDirty(DIRTY_CONSOLE);
			_console_shown = false;
		}
	}

	bool consoleShown( void ) const
	{ return _console_shown; }

	void setStats( WindowRefPtr win )
	{ _stats = win; }

	void showStats( void )
	{
		if( !_stats_shown )
		{
			setDirty(DIRTY_STATS);
			_stats_shown = true;
		}
	}

	void hideStats( void )
	{
		if( _stats_shown )
		{
			setDirty(DIRTY_STATS);
			_stats_shown = false;
		}
	}

	bool statsShown( void ) const
	{ return _stats_shown; }

	void setLoadingScreen( WindowRefPtr win )
	{ _loading_screen = win; }

	void showLoadingScreen( void )
	{
		if( !_loading_screen_shown )
		{
			setDirty(DIRTY_LOADING_SCREEN);
			_loading_screen_shown = true;
		}
	}

	void hideLoadingScreen( void )
	{
		if( _loading_screen_shown )
		{
			setDirty(DIRTY_LOADING_SCREEN);
			_loading_screen_shown = false;
		}
	}

	bool loadingScreenShown( void ) const
	{ return _loading_screen_shown; }

	bool shown( void ) const
	{ return consoleShown() || editorShown(); }

	enum DirtyBits
	{
		DIRTY_EDITOR = Distributed::DIRTY_CUSTOM << 0,
		DIRTY_CONSOLE = Distributed::DIRTY_CUSTOM << 1,
		DIRTY_STATS = Distributed::DIRTY_CUSTOM << 2,
		DIRTY_LOADING_SCREEN = Distributed::DIRTY_CUSTOM << 3,
		DIRTY_CUSTOM = Distributed::DIRTY_CUSTOM << 4,
	};

/// Private Methods
private :
	virtual void serialize( cluster::ByteStream &msg, const uint64_t dirtyBits )
	{
		if( DIRTY_EDITOR & dirtyBits )
		{
			msg << _editor_shown;
		}

		if( DIRTY_CONSOLE & dirtyBits )
		{
			msg << _console_shown;
		}

		if( DIRTY_STATS & dirtyBits )
		{
			msg << _stats_shown;
		}

		if( DIRTY_LOADING_SCREEN & dirtyBits )
		{
			msg << _loading_screen_shown;
		}
	}

	virtual void deserialize( cluster::ByteStream &msg, const uint64_t dirtyBits )
	{
		if( DIRTY_EDITOR & dirtyBits )
		{
			msg >> _editor_shown;
			if( _editor )
			{ _editor->setVisible(_editor_shown); }
		}

		if( DIRTY_CONSOLE & dirtyBits )
		{
			msg >> _console_shown;
			if( _console )
			{ _console->setVisible(_console_shown); }
		}

		if( DIRTY_STATS & dirtyBits )
		{
			msg >> _stats_shown;
			if( _stats )
			{ _stats->setVisible(_stats_shown); }
		}

		if( DIRTY_LOADING_SCREEN & dirtyBits )
		{
			msg >> _loading_screen_shown;
			if( _loading_screen )
			{ _loading_screen->setVisible(_loading_screen_shown); }
		}
	}

/// Data
private :
	bool _editor_shown;
	bool _console_shown;
	bool _stats_shown;
	bool _loading_screen_shown;

	vl::gui::WindowRefPtr _editor;
	vl::gui::WindowRefPtr _console;
	vl::gui::WindowRefPtr _stats;
	vl::gui::WindowRefPtr _loading_screen;

};	// class GUI

/// Actions
class GUIActionBase
{
public :
	GUIActionBase( void )
		: _gui(0)
	{}

	void setGUI( GUI *gui )
	{ _gui = gui; }

	GUI *getGUI( void )
	{ return _gui; }

protected :
	GUI *_gui;

};

class HideEditor : public BasicAction, public GUIActionBase
{
public :
	virtual void execute( void )
	{
		assert( _gui );
		_gui->hideEditor();
	}

	virtual std::string getTypeName( void ) const
	{ return "HideEditor"; }

	static HideEditor *create( void )
	{ return new HideEditor; }
};

class ShowEditor : public BasicAction, public GUIActionBase
{
public :
	virtual void execute( void )
	{
		assert( _gui );
		_gui->showEditor();
	}

	virtual std::string getTypeName( void ) const
	{ return "ShowEditor"; }

	static ShowEditor *create( void )
	{ return new ShowEditor; }
};

class HideConsole : public BasicAction, public GUIActionBase
{
public :
	virtual void execute( void )
	{
		assert( _gui );
		_gui->hideConsole();
	}

	virtual std::string getTypeName( void ) const
	{ return "HideConsole"; }

	static HideConsole *create( void )
	{ return new HideConsole; }
};

class ShowConsole : public BasicAction, public GUIActionBase
{
public :
	virtual void execute( void )
	{
		assert( _gui );
		_gui->showConsole();
	}

	virtual std::string getTypeName( void ) const
	{ return "ShowConsole"; }

	static ShowConsole *create( void )
	{ return new ShowConsole; }
};

}	// namespace gui

}	// namespace vl

#endif	// VL_GUI_GUI_HPP
