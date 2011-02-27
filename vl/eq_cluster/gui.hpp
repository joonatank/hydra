/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-02
 *	@file gui.hpp
 *
 */
#ifndef VL_GUI_HPP
#define VL_GUI_HPP

#include <distributed.hpp>
#include <session.hpp>
#include <action.hpp>

namespace vl
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
	{
		session->registerObject(this, OBJ_GUI, id);
	}

	void showEditor( void )
	{
		setDirty(DIRTY_EDITOR);
		_editor_shown = true;
	}

	void hideEditor( void )
	{
		setDirty(DIRTY_EDITOR);
		_editor_shown = false;
	}

	bool editorShown( void ) const
	{ return _editor_shown; }

	void showConsole( void )
	{
		setDirty(DIRTY_CONSOLE);
		_console_shown = true;
	}

	void hideConsole( void )
	{
		setDirty(DIRTY_CONSOLE);
		_console_shown = false;
	}

	bool consoleShown( void ) const
	{ return _console_shown; }

	bool shown( void ) const
	{ return consoleShown() || editorShown(); }

	enum DirtyBits
	{
		DIRTY_EDITOR = Distributed::DIRTY_CUSTOM << 0,
		DIRTY_CONSOLE = Distributed::DIRTY_CUSTOM << 1,
		DIRTY_CUSTOM = Distributed::DIRTY_CUSTOM << 2,
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
	}

	virtual void deserialize( cluster::ByteStream &msg, const uint64_t dirtyBits )
	{
		if( DIRTY_EDITOR & dirtyBits )
		{
			msg >> _editor_shown;
		}

		if( DIRTY_CONSOLE & dirtyBits )
		{
			msg >> _console_shown;
		}
	}

/// Data
private :
	bool _editor_shown;
	bool _console_shown;
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

}

#endif	// VL_GUI_HPP
