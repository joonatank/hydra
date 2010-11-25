#ifndef EQOGRE_WGLWINDOW_HPP
#define EQOGRE_WGLWINDOW_HPP

#include <eq/client/wglWindow.h>

namespace eqOgre
{

class WGLWindow : public eq::WGLWindow
{
public :
	WGLWindow( eq::Window *parent )
		: eq::WGLWindow(parent)
	{}
	
	virtual ~WGLWindow( void ) {}

	/// These need to be empty for us to use our own OIS event handling
	/// There is not equivalent functions in wglPipe so we don't need to
	/// override those (unlike in GLXWindow/GLXPipe).
	virtual void initEventHandler( void )
	{ return; }

	virtual void exitEventHandler( void )
	{ return; }
};

}

#endif // EQOGRE_WGLWINDOW_HPP
