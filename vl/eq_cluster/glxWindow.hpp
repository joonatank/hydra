#ifndef EQOGRE_GLXWINDOW_HPP
#define EQOGRE_GLXWINDOW_HPP

#include <eq/client/glXWindow.h>

namespace eqOgre
{


namespace
{
static Bool WaitForNotify( Display*, XEvent *e, char *arg )
{ return (e->type == MapNotify) && (e->xmap.window == (::Window)arg); }
}

class GLXWindow : public eq::GLXWindow
{
public :
	GLXWindow( eq::Window *parent )
		: eq::GLXWindow(parent)
	{}
	
	virtual ~GLXWindow( void ) {}

	/// These need to be empty for us to use our own OIS event handling
	/// Remember also that the same functions in GLXPipe need to be empty
	virtual void initEventHandler( void )
	{ return; }

	virtual void exitEventHandler( void )
	{ return; }

	virtual bool configInitGLXWindow( XVisualInfo* visualInfo )
	{
		if( !getXDisplay() )
		{
			_window->setErrorMessage( "Pipe has no X11 display connection" );
			return false;
		}

		eq::PixelViewport pvp = _window->getPixelViewport();
		if( getIAttribute( eq::Window::IATTR_HINT_FULLSCREEN ) == eq::ON )
		{
			const int screen = DefaultScreen( getXDisplay() );

			pvp.h = DisplayHeight( getXDisplay(), screen );
			pvp.w = DisplayWidth( getXDisplay(), screen );
			pvp.x = 0;
			pvp.y = 0;

			_window->setPixelViewport( pvp );
		}

		XID drawable = _createWindow( visualInfo, pvp );
		if( !drawable )
			return false;

		// map and wait for MapNotify event
		XMapWindow( getXDisplay(), drawable );

		XEvent event;
		XIfEvent( getXDisplay(), &event, WaitForNotify, (XPointer)(drawable) );

		XMoveResizeWindow( getXDisplay(), drawable, pvp.x, pvp.y, pvp.w, pvp.h );
		XFlush( getXDisplay() );

		// Grab keyboard focus in fullscreen mode
		if( getIAttribute( eq::Window::IATTR_HINT_FULLSCREEN ) == eq::ON )
			XGrabKeyboard( getXDisplay(), drawable, True, GrabModeAsync, GrabModeAsync,
						CurrentTime );

		setXDrawable( drawable );

		EQINFO << "Created X11 drawable " << drawable << std::endl;
		return true;
	}

private :
	XID _createWindow( XVisualInfo* visualInfo , const eq::PixelViewport& pvp )
	{
		EQASSERT( getIAttribute( eq::Window::IATTR_HINT_DRAWABLE ) != eq::PBUFFER );

		if( !visualInfo )
		{
			_window->setErrorMessage( "No visual info given" );
			return 0;
		}

		if( !getXDisplay() )
		{
			_window->setErrorMessage( "Pipe has no X11 display connection" );
			return 0;
		}

		const int            screen = DefaultScreen( getXDisplay() );
		XID                  parent = RootWindow( getXDisplay(), screen );
		XSetWindowAttributes wa;
		wa.colormap          = XCreateColormap( getXDisplay(), parent, visualInfo->visual,
												AllocNone );
		wa.background_pixmap = None;
		wa.border_pixel      = 0;
		wa.event_mask        = StructureNotifyMask | VisibilityChangeMask |
							   ExposureMask;

		if( getIAttribute( eq::Window::IATTR_HINT_DECORATION ) != eq::OFF )
			wa.override_redirect = False;
		else
			wa.override_redirect = True;

		XID drawable = XCreateWindow( getXDisplay(), parent,
									pvp.x, pvp.y, pvp.w, pvp.h,
									0, visualInfo->depth, InputOutput,
									visualInfo->visual,
									CWBackPixmap | CWBorderPixel |
									CWEventMask | CWColormap | CWOverrideRedirect,
									&wa );

		if ( !drawable )
		{
			_window->setErrorMessage( "Could not create window" );
			return 0;
		}

		std::stringstream windowTitle;
		const std::string& name = "eqOgre";//_window->getName();

		if( name.empty( ))
		{
			windowTitle << "Equalizer";
#ifndef NDEBUG
			windowTitle << " (" << getpid() << ")";
#endif
		}
		else
			windowTitle << name;

		XStoreName( getXDisplay(), drawable, windowTitle.str().c_str( ));

		// Register for close window request from the window manager
		Atom deleteAtom = XInternAtom( getXDisplay(), "WM_DELETE_WINDOW", False );
		XSetWMProtocols( getXDisplay(), drawable, &deleteAtom, 1 );

		return drawable;
	}

};

}

#endif // EQOGRE_GLXWINDOW_HPP