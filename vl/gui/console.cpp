/**
 *	Copyright (c) 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-02
 *	@file gui/console.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 */

// Interface
#include "console.hpp"

// Necessary for passing messages back to creator
#include "gui.hpp"

#define CONSOLE_FONT_INDEX 14

#define CONSOLE_LINE_LENGTH 85
#define CONSOLE_LINE_COUNT 15


bool isValidChar(unsigned int c)
{
	static const char legalchars[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890+!\"'#%&/()=?[]\\*-_.:,; ";
	// needs to be skipped as their text field is legal and appears as a whitespace
	// otherwise doesn't skip special characters
	// shift, menu, meta, control
	if(c == 0)
	{ return false; }

	for(int i=0; i < sizeof(legalchars); i++)
	{
		if(legalchars[i] == c)
		{
			return true;
		}
	}
	return false;
}

/// ---------------------------- ConsoleWindow ------------------------------
/// ------------------------------ Public -----------------------------------
vl::gui::ConsoleWindow::ConsoleWindow(vl::gui::GUI *creator)
	: Window(creator, "")
	, _console_memory_index(-1)	// Using -1 index to indicate not using memory
	, _py_error_colour(Ogre::ColourValue(0.5, 0.2, 0))
	, _py_out_colour(Ogre::ColourValue(0, 0.5, 0.5))
	, _error_colour(Ogre::ColourValue(0.5, 0, 0))
	, _out_colour(Ogre::ColourValue(0.2, 0.4, 0.8))
	, mStartline(0)
	, mUpdateConsole(false)
	, mUpdatePrompt(false)
	, mIsInitialised(false)
	, _caret_position(0)
{
}

vl::gui::ConsoleWindow::~ConsoleWindow(void)
{
	shutdown();
}

void
vl::gui::ConsoleWindow::shutdown(void)
{
	if(!mIsInitialised)
		return;

	mIsInitialised = false;

	mScreen->destroy(mLayer);
}

void
vl::gui::ConsoleWindow::injectKeyDown(OIS::KeyEvent const &arg)
{
	if(!_visible)
		return;

	// @todo add modifier support
	// e.g. CTRL, ALT and SHIFT
	// SHIFT should be passed to the text reader if not consumed
	// ALT and CTRL should not be passed
	// Primarily add CTRL+HOME and CTRL+END

	if (arg.key == OIS::KC_RETURN || arg.key == OIS::KC_NUMPADENTER)
	{
		_input_accepted();
	}
	else if (arg.key == OIS::KC_BACK)
	{
		if(_caret_position > 0)
		{
			_prompt.erase(_prompt.begin()+_caret_position-1);
			_caret_position--;
			mUpdatePrompt = true;
		}
	}
	else if(arg.key == OIS::KC_DELETE)
	{
		if(_caret_position < _prompt.size())
		{
			_prompt.erase(_prompt.begin()+_caret_position);
			mUpdatePrompt = true;
		}
	}
	else if (arg.key == OIS::KC_PGUP)
	{
		if(mStartline>0)
		{ mStartline--; }
		mUpdateConsole = true;
	}
	else if (arg.key == OIS::KC_PGDOWN)
	{
		if(mStartline < _lines.size())
		{ mStartline++; }
		mUpdateConsole = true;
	}
	else if(arg.key == OIS::KC_UP)
	{
		// Save the current user input when the list has not been scrolled
		if( _console_memory_index == -1 )
		{
			_console_last_command = _prompt;
		}

		++_console_memory_index;
		if( _console_memory_index >= _console_memory.size() )
		{ _console_memory_index = _console_memory.size()-1; }

		if( _console_memory_index > -1 )
		{
			_prompt = _console_memory.at(_console_memory_index);
		}
		_caret_position = _prompt.size();
	}
	else if(arg.key == OIS::KC_DOWN)
	{
		--_console_memory_index;
		if( _console_memory_index < 0 )
		{
			_console_memory_index = -1;
			_prompt = _console_last_command;
		}
		else
		{
			_prompt = _console_memory.at(_console_memory_index);
		}
		_caret_position = _prompt.size();
	}
	else if(arg.key == OIS::KC_LEFT)
	{
		if(_caret_position > 0)
		{ _caret_position--; }
		mUpdatePrompt = true;
	}
	else if(arg.key == OIS::KC_RIGHT)
	{
		if(_caret_position < _prompt.size())
		{ ++_caret_position; }
		mUpdatePrompt = true;
	}
	else if(arg.key == OIS::KC_END)
	{
		_caret_position = _prompt.size();
		mUpdatePrompt = true;
	}
	else if(arg.key == OIS::KC_HOME)
	{
		_caret_position = 0;
		mUpdatePrompt = true;
	}
	else
	{
		if(isValidChar(arg.text))
		{
			_prompt.insert(_caret_position, 1, char(arg.text));
			++_caret_position;
		}
	}

	mUpdatePrompt = true;
}

void
vl::gui::ConsoleWindow::injectKeyUp(OIS::KeyEvent const &key)
{
}

void
vl::gui::ConsoleWindow::printTo(std::string const &text, double time,
						 std::string const &type, vl::LOG_MESSAGE_LEVEL lvl)
{
	/* @todo colours not yet supported
	if( type == "OUT" )
	{
		item->setTextColours(_out_colour);
	}
	else if( type == "ERROR" )
	{
		item->setTextColours(_error_colour);
	}
	else if( type == "PY_OUT" )
	{
		item->setTextColours(_py_out_colour);
	}
	else if( type == "PY_ERROR" )
	{
		item->setTextColours(_py_error_colour);
	}
	*/

	_print(text);
}

void
vl::gui::ConsoleWindow::_input_accepted(void)
{
	if(!mIsInitialised)
	{ return; }

	std::string command(_prompt);

	printTo("%3> " + _prompt + "%R", double(0));

	if( command.size() > 0 )
	{
		if( *(command.end()-1) == ':' )
		{
			std::string str("Multi Line commands are not supported yet.");
			printTo(str, 0);
		}
		else
		{
			while( _console_memory.size() > 100 )
			{ _console_memory.pop_back(); }

			_console_memory.push_front(command);

			_command_signal(command);
		}

		// Reset the memory index because the user has accepted the command
		_console_memory_index = -1;
		_console_last_command.clear();
	}

	mUpdateConsole = true;
	mUpdatePrompt = true;
	_prompt.clear();
	_caret_position = 0;
}

/// ------------------------------ Private -----------------------------------
void
vl::gui::ConsoleWindow::_window_resetted(void)
{
	if(mIsInitialised)
		shutdown();

	assert(mScreen);

	// Create gorilla things here.
	mLayer = mScreen->createLayer(15);
	mGlyphData = mLayer->_getGlyphData(CONSOLE_FONT_INDEX);

	mConsoleText = mLayer->createMarkupText(CONSOLE_FONT_INDEX,  10,10, Ogre::StringUtil::BLANK);
	mConsoleText->width(mScreen->getWidth() - 10);
	mPromptText = mLayer->createCaption(CONSOLE_FONT_INDEX,  10,10, "> _");
	mDecoration = mLayer->createRectangle(8,8, mScreen->getWidth() - 16, mGlyphData->mLineHeight );
	mDecoration->background_gradient(Gorilla::Gradient_NorthSouth, Gorilla::rgb(128,128,128,128), Gorilla::rgb(64,64,64,128));
	mDecoration->border(2, Gorilla::rgb(128,128,128,128));
   
	mIsInitialised = true;
}

void
vl::gui::ConsoleWindow::_update(void)
{
	if(!mIsInitialised)
	{ return; }

	if(mLayer->isVisible() != _visible)
	{ mLayer->setVisible(_visible); }

	if(mUpdateConsole)
		_updateConsole();
    
	if (mUpdatePrompt)
		_updatePrompt();
}

void
vl::gui::ConsoleWindow::_updateConsole(void)
{
	if(!mIsInitialised)
	{ return; }

	mUpdateConsole = false;
 
	std::stringstream text;
	std::list<Ogre::String>::iterator i, start, end;

	//make sure is in range
	if(mStartline > _lines.size())
	{ mStartline = _lines.size(); }

	int lcount=0;
	start = _lines.begin();
	for(int c=0; c < mStartline; c++)
	{ start++; }

	end=start;
	for(int c=0; c < CONSOLE_LINE_COUNT; c++)
	{
		if(end==_lines.end())
			break;
		end++;
	}

	for(i=start; i!=end; i++)
	{
		lcount++;
		text << (*i) << "\n";
	}
	mConsoleText->text(text.str());
 
	// Move prompt downwards.
	mPromptText->top(10 + (lcount * mGlyphData->mLineHeight));
 
	// Change background height so it covers the text and prompt
	mDecoration->height(((lcount+1) * mGlyphData->mLineHeight) + 4);
 
	mConsoleText->width(mScreen->getWidth() - 20);
	mDecoration->width(mScreen->getWidth() - 16);
	mPromptText->width(mScreen->getWidth() - 20);
}

void
vl::gui::ConsoleWindow::_updatePrompt(void)
{
	if(!mIsInitialised)
	{ return; }

	mUpdatePrompt = false;
	std::stringstream text;
	text << "> " << _prompt.substr(0, _caret_position) << "_" << _prompt.substr(_caret_position);
	mPromptText->text(text.str());
}

void
vl::gui::ConsoleWindow::_print(Ogre::String const &text)
{
	//subdivide it into lines
	const char *str = text.c_str();
	int start=0, count=0;
	int len = text.length();
	Ogre::String line;
	for(int c=0; c < len; c++)
	{
		if(str[c]=='\n'||line.length() >= CONSOLE_LINE_LENGTH)
		{
			_lines.push_back(line);
			line="";
		}
		if(str[c]!='\n')
		{ line += str[c]; }
	}
	if(line.length())
	{ _lines.push_back(line); }
	if(_lines.size() > CONSOLE_LINE_COUNT)
	{ mStartline = _lines.size() - CONSOLE_LINE_COUNT; }
	else
	{ mStartline = 0; }
	mUpdateConsole = true;
}
