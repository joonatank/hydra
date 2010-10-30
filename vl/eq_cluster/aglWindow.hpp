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

	/// These need to be empty for us to use our own OIS event handling
	/// Remember also that the same functions in AGLPipe need to be empty
	virtual void initEventHandler( void )
	{ return; }

	virtual void exitEventHandler( void )
	{ return; }

};

}

#endif // EQOGRE_AGLWINDOW_HPP
