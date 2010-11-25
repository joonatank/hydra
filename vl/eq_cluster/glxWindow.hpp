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

	/// We override window creation because we use the XDisplay created by OIS
	/// for event handling and the OIS XDisplay does not get all Keyboard
	/// events if we don't disable those in here.
	/// Disabling event receiving after window creation seems not to work.
	virtual bool configInitGLXWindow( XVisualInfo* visualInfo );


private :
	XID _createWindow( XVisualInfo* visualInfo , const eq::PixelViewport& pvp );

};

}

#endif // EQOGRE_GLXWINDOW_HPP
