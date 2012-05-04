
#ifndef OGRE_GL_WINDOW_HPP
#define OGRE_GL_WINDOW_HPP

// Base class
#include <OGRE/OgreRenderWindow.h>

#include "OgreGLSupport.h"

namespace Ogre {

// Base class for platform GLWindows
class _OgreGLExport GLWindow : public RenderWindow
{
public:
	GLWindow(void) {}

	virtual ~GLWindow(void) {}

	static GLWindow *createWindow(GLSupport &support);

};	// class GLWindow

}	// namespace Ogre

#endif	// OGRE_GL_WINDOW_HPP
