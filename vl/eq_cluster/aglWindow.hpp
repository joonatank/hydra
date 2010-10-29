#ifndef EQOGRE_AGLWINDOW_HPP
#define EQOGRE_AGLWINDOW_HPP

#include <eq/client/aglWindow.h>

namespace eqOgre
{

class AGLWindow : public eq::AGLWindow
{
public :
	AGLWindow( eq::Window *parent )
		: eq::AGLWindow(parent)
	{}

	virtual ~AGLWindow( void ) {}

	void initEventHandler( void )
	{ return; }

};

}

#endif // EQOGRE_AGLWINDOW_HPP
