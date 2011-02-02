/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2011-01
 *
 *
 */

#ifndef EQOGRE_GLXWINDOW_HPP
#define EQOGRE_GLXWINDOW_HPP

// Base class
#include "glWindow.hpp"

#include <X11/Xlib.h>

namespace eqOgre
{

class GLXWindow : public GLWindow
{
public :
	GLXWindow( void );

	virtual ~GLXWindow( void );

	XID getXID( void ) const
	{ return _drawable; }

	Display *getXDisplay( void )
	{ return _xdisp; }

private :
	XID _createWindow( int x, int y, unsigned int w, unsigned int h );

	Display *_xdisp;
	XID _drawable;
};

}

#endif // EQOGRE_GLXWINDOW_HPP
