
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

	// Override RenderTarget::update
	// We use a FBO to render the real scene and then render that scene
	// to the window using a quad.
	// This will later be extended to grabbing other FBO inputs that are filled
	// elsewhere.
	// Also we will provide a stereo camera support, using two user set cameras
	// that are passed as misc parameters.
	virtual void update(bool swapBuffers = true);

};	// class GLWindow

}	// namespace Ogre

#endif	// OGRE_GL_WINDOW_HPP
