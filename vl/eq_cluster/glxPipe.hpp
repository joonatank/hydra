#ifndef EQOGRE_GLXPIPE_HPP
#define EQOGRE_GLXPIPE_HPP

#include <eq/client/glXPipe.h>

namespace eqOgre
{

class GLXPipe : public eq::GLXPipe
{
public :
	GLXPipe( eq::Pipe *parent )
		: eq::GLXPipe(parent)
	{}

	virtual ~GLXPipe( void ) {}

	/// These need to be empty for us to use our own OIS event handling
	/// Remember also that the same functions in GLXWindow need to be empty
	/// or the application will crash (segfault because invalid event handler here).
	virtual void initEventHandler( void )
	{ return; }

	virtual void exitEventHandler( void )
	{ return; }

};

}

#endif // EQOGRE_GLXPIPE_HPP
