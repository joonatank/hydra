/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-02
 *	@file window.hpp
 *
 */

#ifndef VL_GUI_WINDOW_HPP
#define VL_GUI_WINDOW_HPP

#include <CEGUI/CEGUIWindow.h>

#include <boost/shared_ptr.hpp>

namespace vl
{

namespace gui
{
/** @class Window
 *	@brief Wrapper around CEGUI Window, provides the callbacks
 */
class Window
{
public :
	Window( CEGUI::Window *win )
		: _window(win)
	{}

	void setVisible( bool visible )
	{
		_window->setVisible(visible);
	}

protected :
	CEGUI::Window *_window;
};

typedef boost::shared_ptr<Window> WindowRefPtr;

}	// namespace gui

}	// namespace vl

#endif	// VL_GUI_WINDOW_HPP
