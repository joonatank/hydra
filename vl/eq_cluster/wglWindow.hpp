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
	virtual void initEventHandler()
	{ return; }

	virtual void exitEventHandler()
	{ return; }
};

}

#endif // EQOGRE_WGLWINDOW_HPP
