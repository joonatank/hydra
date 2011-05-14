/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-05
 *	@file gui_actions.hpp
 *
 *	This file is part of Hydra VR game engine.
 */

#ifndef HYDRA_GUI_ACTIONS_HPP
#define HYDRA_GUI_ACTIONS_HPP

// Base actions
#include "action.hpp"
// Necessary for RefPtrs
#include "typedefs.hpp"

#include "gui.hpp"

namespace vl
{

namespace gui
{

class GUIActionBase
{
public :
	GUIActionBase(void)
	{}

	void setGUI( GUIRefPtr gui )
	{ _gui = gui; }

	GUIRefPtr getGUI( void )
	{ return _gui; }

protected :
	GUIRefPtr _gui;

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

#endif	// HYDRA_GUI_ACTIONS_HPP
