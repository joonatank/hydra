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

#include <GL/glx.h>

namespace eqOgre
{

class Window;

class GLXWindow : public GLWindow
{
public :
	GLXWindow( eqOgre::Window *win );

	virtual ~GLXWindow( void );

	XID getXID( void ) const
	{ return _drawable; }

	Display *getXDisplay( void )
	{ return _xdisp; }

	virtual void makeCurrent( void ) const;

private :
	XID _createWindow( int x, int y, unsigned int w, unsigned int h );

	GLXContext createGLXContext( XVisualInfo *visuInfo );

	// TODO add attributes
	XVisualInfo *_chooseVisual( void );

	eqOgre::Window *_window;

	Display *_xdisp;
	XID _drawable;
	GLXContext _glContext;

};	// class GLXWindow

}

#endif // EQOGRE_GLXWINDOW_HPP
