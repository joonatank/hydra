#ifndef EQOGRE_GLXWINDOW_HPP
#define EQOGRE_GLXWINDOW_HPP

#include <eq/client/glXWindow.h>

namespace eqOgre
{
	
class GLXWindow : public eq::GLXWindow
{
public :
	GLXWindow( eq::Window *parent )
		: eq::GLXWindow(parent)
	{}
	
	virtual ~GLXWindow( void ) {}
	
	void initEventHandler( void )
	{ return; }

};

}

#endif // EQOGRE_GLXWINDOW_HPP
