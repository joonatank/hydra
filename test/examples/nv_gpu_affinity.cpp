/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-06
 *	@file nv_swap_sync.cpp
 *
 *	This file is part of Hydra a VR game engine tests.
 *
 *	Test the usage of nvidia GPU affinity
 *
 *	Needs OpenGL 3.0
 *
 *	Works only on a recent Quadro GPU.
 *	Works only on Windows.
 *
 */


#include "gl_common.hpp"

#include "gl_fbo.hpp"

#include <fstream>

#define MAX_GPU 4

GLContext *createAffinityContext(uint16_t gpu_id)
{
	PIXELFORMATDESCRIPTOR pfd;
	int    pf, gpuIndex = 0;
	HGPUNV hGPU[MAX_GPU];
	HGPUNV GpuMask[MAX_GPU];
	HDC    affDC;
	HGLRC  affRC;

	// Get a list of the first MAX_GPU GPUs in the system
	while( (gpuIndex < MAX_GPU) && wglEnumGpusNV(gpuIndex, &hGPU[gpuIndex]) )
	{ gpuIndex++; }

	if(gpu_id >= gpuIndex)
	{
		MessageBox(NULL, "Trying to create affinity context to an GPU that does not exist.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
		return 0;
	}

	// Create an affinity-DC associated with the first GPU
	GpuMask[0] = hGPU[gpu_id];
	GpuMask[1] = NULL;

	affDC = wglCreateAffinityDCNV(GpuMask);

	// Set a pixelformat on the affinity-DC
	pf = ChoosePixelFormat(affDC, &pfd);
	SetPixelFormat(affDC, pf, &pfd);
	DescribePixelFormat(affDC, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

	return new GLContext(affDC); //affRC = wglCreateContext(affDC);
}

int WINAPI WinMain(	HINSTANCE	hInstance,			// Instance
					HINSTANCE	hPrevInstance,		// Previous Instance
					LPSTR		lpCmdLine,			// Command Line Parameters
					int			nCmdShow)			// Window Show State
{
	uint32_t width = 640;
	uint32_t height = 480;

	// Create Our OpenGL Window
	GLWindow *win = GLWindow::create("NV GPU affinity test", width, height, 16);
	if(!win)
	{
		MessageBox(NULL, "OpenGL window creation failed.", "ERROR", MB_OK|MB_ICONEXCLAMATION); 
		return 0;
	}

	GLenum err = glewInit();
	if(GLEW_OK != err)
	{
		MessageBox(NULL, "GLEW init failed.", "ERROR", MB_OK|MB_ICONEXCLAMATION); 
		return -1;
	}

	if(!GLEW_VERSION_3_0)
	{
		MessageBox(NULL, "The Copy image demo needs OpenGL 3.0.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
		return -1;
	}
	if(!WGLEW_NV_copy_image)
	{
		MessageBox(NULL, "The Copy image demo needs NV_copy_image extension.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
		return -1;
	}
	if(!WGLEW_NV_gpu_affinity)
	{
		MessageBox(NULL, "The GPU affinity image demo needs NV_gpu_affinity extension.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
		return -1;
	}

	std::ofstream log_file("debug.log");

	log_file << "Starting NVidia GPU affinity rendering test" << std::endl;

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

	// Create an Affinity Context for the GPU and create the FBO on that context
	uint16_t gpu_id = 1;
	GLContext *aff_context = createAffinityContext(gpu_id);
	if(!aff_context)
	{ return -1; }
	if(!aff_context->makeCurrent())
	{ return -1; }

	InitGL();

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
					aff_context->makeCurrent();
					fbo->beginDraw();

					glMatrixMode(GL_PROJECTION);
					glLoadIdentity();
					gluPerspective(45.0f, (GLfloat)width/(GLfloat)height, 0.1f, 100.0f);					

					glClearColor(0.0f, 0.0f, 1.0f, 0.0f);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear The Screen And The Depth Buffer

					DrawGLScene();
					
					fbo->endDraw();

					// Draw the FBO to the Window
					// needs a copy image and a texture in the window context where to copy
					if( !wglCopyImageSubDataNV(aff_context->getGLContext(), fbo->getTexture(), 
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
	delete aff_context;
	delete win;			// Kill The Window
	delete fbo;
	return 0;			// Exit The Program
}
