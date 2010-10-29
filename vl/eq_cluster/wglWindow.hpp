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

	void initEventHandler( void )
	{ return; }

};

}

#endif // EQOGRE_WGLWINDOW_HPP
