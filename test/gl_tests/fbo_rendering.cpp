/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-06
 *	@file fbo_rendering.cpp
 *
 *	This file is part of Hydra a VR game engine tests.
 *
 *	Needs OpenGL 3.0
 *
 *	Test the FBO rendering.
 *	Works only on Windows.
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
	GLWindow *win = GLWindow::create("OpenGL FBO rendering test", width, height, 16);
	if(!win)
	{
		MessageBox(NULL, "OpenGL window creation failed.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
		return 0;
	}

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		MessageBox(NULL, "GLEW init failed.", "ERROR", MB_OK|MB_ICONEXCLAMATION); 
		return 0;
	}

	if(!GLEW_VERSION_3_0)
	{
		MessageBox(NULL, "The FBO demo needs OpenGL 3.0.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
		return 0;
	}

	std::ofstream log_file("debug.log");

	log_file << "Starting FBO rendering test" << std::endl;

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
					fbo->beginDraw();

					glMatrixMode(GL_PROJECTION);
					glLoadIdentity();
					gluPerspective(45.0f, (GLfloat)width/(GLfloat)height, 0.1f, 100.0f);					

					glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear The Screen And The Depth Buffer

					DrawGLScene();
					
					fbo->endDraw();


					// Draw the FBO to the Window
					fbo->drawToScreen();

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
