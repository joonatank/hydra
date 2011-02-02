
// Interface
#include "glxWindow.hpp"

// Necessary for getPid
#include "base/system_util.hpp"

#include <assert.h>
#include <iostream>
#include <sstream>

namespace
{
static Bool WaitForNotify( Display*, XEvent *e, char *arg )
{ return (e->type == MapNotify) && (e->xmap.window == (::Window)arg); }
}

eqOgre::GLXWindow::GLXWindow( void )
	: _xdisp(0), _drawable(0)
{
	// TODO this should support the correct display name
	_xdisp = XOpenDisplay(NULL);
	if( NULL == _xdisp )
	{
		// TODO this should throw
		return;
	}

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
