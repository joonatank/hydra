
// Interface
#include "OgreGLWindow.hpp"

// Platform headers
#ifdef _WIN32
#include "OgreWin32Window.h"
// Necessary for casting the support
#include "OgreWin32GLSupport.h"
#else
#error "Linux and OSX not yet supported."
#endif

#ifdef _WIN32
Ogre::GLWindow *Ogre::GLWindow::createWindow(GLSupport &support)
{
	return new Win32Window((Win32GLSupport &)support);
}
#endif
