
// Interface
#include "glxWindow.hpp"

// Necessary for getPid
#include "base/system_util.hpp"

#include "window.hpp"

#include <assert.h>
#include <iostream>
#include <sstream>

namespace
{
static Bool WaitForNotify( Display*, XEvent *e, char *arg )
{ return (e->type == MapNotify) && (e->xmap.window == (::Window)arg); }
}

eqOgre::GLXWindow::GLXWindow( eqOgre::Window *win )
	: _window(win), _xdisp(0), _drawable(0), _glContext(0)
{
	// TODO this should support the correct display name
	_xdisp = XOpenDisplay(NULL);
	if( NULL == _xdisp )
	{
		// TODO this should throw
		return;
	}

	XVisualInfo *visuInfo = _chooseVisual();
	assert( visuInfo );

	// TODO the window position and size needs to be passed here
	XID drawable = _createWindow( 0, 0, 1024, 768 );
	if( !drawable )
	{
		// TODO this should throw
		return;
	}

	// map and wait for MapNotify event
	XMapWindow( getXDisplay(), drawable );

	XEvent event;
	XIfEvent( getXDisplay(), &event, WaitForNotify, (XPointer)(drawable) );

 	XMoveResizeWindow( getXDisplay(), drawable, 0, 0, 1024, 768 );
	XFlush( getXDisplay() );

	_drawable = drawable;

	_glContext = createGLXContext( visuInfo );

	std::cout << "Created X11 drawable " << drawable << std::endl;
}

eqOgre::GLXWindow::~GLXWindow(void )
{
	// TODO cleanup
}

XID
eqOgre::GLXWindow::_createWindow( int x, int y, unsigned int w, unsigned int h )
{
	assert( NULL != getXDisplay() );

	const int            screen = DefaultScreen( getXDisplay() );
	XID                  parent = RootWindow( getXDisplay(), screen );
	XSetWindowAttributes wa;
	wa.colormap          = DefaultColormap( getXDisplay(), screen );
	wa.background_pixmap = None;
	wa.border_pixel      = 0;
	wa.event_mask        = StructureNotifyMask | VisibilityChangeMask |
							ExposureMask;
	// Remove all decorations
	wa.override_redirect = True;;

	unsigned long int variables = CWBackPixmap | CWBorderPixel |
 								CWEventMask | CWColormap | CWOverrideRedirect;
	int depth = XDefaultDepth( getXDisplay(), screen );

	// TODO remove borders
	XID drawable = XCreateWindow( getXDisplay(), parent,
								x, y, w, h,
								0, depth, InputOutput,
								DefaultVisual( getXDisplay(), screen ),
								variables,
								&wa );

	if ( !drawable )
	{
		// TODO this should throw
		return 0;
	}

	std::stringstream windowTitle;
	// TODO correct name
	std::string name; //getWindow()->getName();

	if( name.empty( ))
	{
		windowTitle << "Hydra";
#ifndef NDEBUG
		windowTitle << " (" << vl::getPid() << ")";
#endif
	}
	else
	{ windowTitle << name; }

	XStoreName( getXDisplay(), drawable, windowTitle.str().c_str( ));

	// Register for close window request from the window manager
	Atom deleteAtom = XInternAtom( getXDisplay(), "WM_DELETE_WINDOW", False );
	XSetWMProtocols( getXDisplay(), drawable, &deleteAtom, 1 );

	return drawable;
}

void
eqOgre::GLXWindow::makeCurrent( void ) const
{
    assert( _xdisp );

    glXMakeCurrent( _xdisp, _drawable, _glContext );
//     GLXWindowIF::makeCurrent();
    if( _glContext )
    {
        std::cerr << "OpenGL error After glXMakeCurrent" << std::endl;
    }
}

GLXContext
eqOgre::GLXWindow::createGLXContext( XVisualInfo *visuInfo )
{
	std::cout << "eqOgre::GLXWindow::createGLXContext" << std::endl;
	assert( _xdisp && visuInfo );

	GLXContext shareCtx = 0;
//     const Window* shareWindow = getWindow()->getSharedContextWindow();
//     const SystemWindow* sysWindow =
//         shareWindow ? shareWindow->getSystemWindow() :0;
//     if( sysWindow )
//     {
//         EQASSERT( dynamic_cast< const GLXWindowIF* >( sysWindow ));
//         const GLXWindowIF* shareGLXWindow = static_cast< const GLXWindow* >(
//                                                 sysWindow );
//         shareCtx = shareGLXWindow->getGLXContext();
//     }

	GLXContext context = glXCreateContext( _xdisp, visuInfo, shareCtx, True );
	if( !context )
	{
		std::cerr << "Failed creating GLX context." << std::endl;
		// TODO should throw
//         setError( ERROR_GLXWINDOW_CREATECONTEXT_FAILED );
		return 0;
	}

	return context;
}

XVisualInfo *
eqOgre::GLXWindow::_chooseVisual( void )
{
	assert( getXDisplay() );
	const int screen = DefaultScreen( getXDisplay() );
	XVisualInfo *info = 0;

	if( !info )
	{
		std::cout << "Trying to choose Visual with Stereo." << std::endl;
		int attribList[] = {
			GLX_DOUBLEBUFFER,
			GLX_STEREO,
			None };

		info = glXChooseVisual( getXDisplay(), screen, attribList );
	}

	if( !info )
	{
		std::cout << "Trying to choose Visual without Stereo." << std::endl;
		int attribList[] = {
			GLX_DOUBLEBUFFER,
			None };

		info = glXChooseVisual( getXDisplay(), screen, attribList );
	}

	if( !info )
	{
		std::cout << "Trying to choose Visual without Stereo and without Double buffering." << std::endl;
		int attribList[] = { None };

		info = glXChooseVisual( getXDisplay(), screen, attribList );
	}

	return info;
}
