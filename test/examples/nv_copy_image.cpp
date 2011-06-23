/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-06
 *	@file nv_copy_image.cpp
 *
 *	This file is part of Hydra a VR game engine tests.
 *
 *	Test the usage of NVidia copy image extension.
 *
 *	Needs OpenGL 3.0
 *
 *	Works only on Windows.
 *	Works only with Quadro GPUs.
 *
 */

#include "gl_common.hpp"

#include "gl_fbo.hpp"

#include <fstream>

int WINAPI WinMain(	HINSTANCE	hInstance,			// Instance
					HINSTANCE	hPrevInstance,		// Previous Instance
					LPSTR		lpCmdLine,			// Command Line Parameters
					int			nCmdShow)			// Window Show State
{
	uint32_t width = 640;
	uint32_t height = 480;

	// Create Our OpenGL Window
	GLWindow *win = GLWindow::create("NV image copy test", width, height, 16);
	if(!win)
	{
		MessageBox(NULL, "OpenGL window creation failed.", "ERROR", MB_OK|MB_ICONEXCLAMATION); 
		return 0;
	}

	GLenum err = glewInit();
	if(GLEW_OK != err)
	{
		MessageBox(NULL, "GLEW init failed.", "ERROR", MB_OK|MB_ICONEXCLAMATION); 
		return 0;
	}

	if(!GLEW_VERSION_3_0)
	{
		MessageBox(NULL, "The Copy image demo needs OpenGL 3.0.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
		return 0;
	}
	if(!WGLEW_NV_copy_image)
	{
		MessageBox(NULL, "The Copy image demo needs NV_copy_image extension.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
		return 0;
	}

	std::ofstream log_file("debug.log");

	log_file << "Starting FBO rendering test" << std::endl;

	// Create the final texture to display in the Window context
	GLuint win_texture;
	glGenTextures(1, &win_texture);

	glBindTexture(GL_TEXTURE_2D, win_texture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	if(glGetError() != GL_NO_ERROR)
	{
		MessageBox(NULL, "Error in creating Window texture", "ERROR", MB_OK|MB_ICONEXCLAMATION);
		return -1;
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	// Setup multiple OpenGL contexts
	// one for the FBO and another one for the window.
	GLContext *fbo_context = new GLContext(win->getDC());
	FBO *fbo = new FBO(width, height);

	bool done = !win;
	while(!done)
	{
		MSG msg;
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))	// Is There A Message Waiting?
		{
			if (msg.message==WM_QUIT)				// Have We Received A Quit Message?
			{
				done=TRUE;							// If So done=TRUE
			}
			else									// If Not, Deal With Window Messages
			{
				TranslateMessage(&msg);				// Translate The Message
				DispatchMessage(&msg);				// Dispatch The Message
			}
		}
		else										// If There Are No Messages
		{
			// Draw The Scene.  Watch For ESC Key And Quit Messages From DrawGLScene()
			if(g_active)								// Program Active?
			{
				if(g_keys[VK_ESCAPE])				// Was ESC Pressed?
				{
					done=TRUE;						// ESC Signalled A Quit
				}
				else								// Not Time To Quit, Update Screen
				{
					// Draw to the FBO
					fbo_context->makeCurrent();
					fbo->beginDraw();

					glMatrixMode(GL_PROJECTION);
					glLoadIdentity();
					gluPerspective(45.0f, (GLfloat)width/(GLfloat)height, 0.1f, 100.0f);					

					glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear The Screen And The Depth Buffer

					DrawGLScene();
					
					fbo->endDraw();

					// Draw the FBO to the Window
					// needs a copy image and a texture in the window context where to copy
					if( !wglCopyImageSubDataNV(fbo_context->getGLContext(), fbo->getTexture(), 
							GL_TEXTURE_2D, 0, 0, 0, 0,
							win->getContext()->getGLContext(), win_texture, 
							GL_TEXTURE_2D, 0, 0, 0, 0, 
							width, height, 1) )
					{
						MessageBox(NULL, "Error in copying fbo image.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
						return -1;
					}
	
					win->makeCurrent();

					drawToScreen(width, height, win_texture);

					win->swapBuffers();
				}
			}
		}
	}

	// Shutdown
	delete win;			// Kill The Window
	delete fbo;
	return 0;			// Exit The Program
}
