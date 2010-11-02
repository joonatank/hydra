
#include "pipe.hpp"

#include "glxPipe.hpp"

eqOgre::Pipe::Pipe( eq::Node *parent )
	: eq::Pipe( parent )
{}

eqOgre::Pipe::~Pipe()
{
}

bool
eqOgre::Pipe::configInit( const uint32_t initID )
{
	EQINFO << "eqOgre::Pipe::configInit" << std::endl;
	if( !eq::Pipe::configInit( initID ))
	{ return false; }

	return true;
}

bool
eqOgre::Pipe::configExit()
{
	return eq::Pipe::configExit();
}

void
eqOgre::Pipe::frameStart( const uint32_t frameID, const uint32_t frameNumber )
{
	eq::Pipe::frameStart( frameID, frameNumber );
}

bool
eqOgre::Pipe::configInitSystemPipe( const uint32_t )
{
	EQINFO << "eqOgre::Pipe::configInitSystemPipe" << std::endl;
	
    eq::SystemPipe* systemPipe = 0;

    switch( getWindowSystem() )
    {
#ifdef GLX
		case eq::WINDOW_SYSTEM_GLX:
            EQINFO << "Using GLXPipe" << std::endl;
            systemPipe = new eqOgre::GLXPipe( this );
            break;
#endif

#ifdef AGL
		case eq::WINDOW_SYSTEM_AGL:
            EQINFO << "Using AGLPipe" << std::endl;
            systemPipe = new eq::AGLPipe( this );
            break;
#endif

#ifdef WGL
		case eq::WINDOW_SYSTEM_WGL:
            EQINFO << "Using WGLPipe" << std::endl;
            systemPipe = new eq::WGLPipe( this );
            break;
#endif

        default:
            EQERROR << "Unknown window system: " << getWindowSystem() << std::endl;
            setErrorMessage( "Unknown window system" );
            return false;
    }

    EQASSERT( systemPipe );
    if( !systemPipe->configInit( ))
    {
        setErrorMessage( "System Pipe initialization failed: " +
            systemPipe->getErrorMessage( ));
        EQERROR << getErrorMessage() << std::endl;
        delete systemPipe;
        return false;
    }

    setSystemPipe( systemPipe );
    return true;
}
