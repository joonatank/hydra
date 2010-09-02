#include "window.hpp"

eqOgre::Window::Window(eq::Pipe *parent)
	: eq::Window( parent ), _ogre_window(0), _camera(0), _sm(0)
{}

eqOgre::Window::~Window(void )
{}

/// Protected
bool
eqOgre::Window::configInit( const uint32_t initID )
{
	if( !eq::Window::configInit( initID ))
	{ return false; }

	return true;
}

void 
eqOgre::Window::createOgreRoot( void )
{
}

void 
eqOgre::Window::createOgreWindow( void )
{
}

void 
eqOgre::Window::createTracker( void )
{
}

bool
eqOgre::Window::createScene( void )
{
	return true;
}

/*
bool
eqOgre::Window::configInitGL( const uint32_t initID )
{
	if( !eq::Window::configInitGL( initID ))
	{ return false; }

    return true;
}

bool
eqOgre::Window::configExitGL()
{
	return eq::Window::configExitGL();
}

void
eqOgre::Window::frameStart( const uint32_t frameID,
		const uint32_t frameNumber )
{
	eq::Window::frameStart( frameID, frameNumber );
}

void
eqOgre::Window::swapBuffers( void )
{
	eq::Window::swapBuffers();
}
*/