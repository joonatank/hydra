/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-06
 *	@file nv_swap_sync.cpp
 *
 *	This file is part of Hydra a VR game engine tests.
 *
 *	Test the usage of NVidia Swap sync.
 *
 *	Works only on Windows.
 *	Works only with Quadro GPUs with G-sync cards using multiple contexes for renderig.
 *	These requirements might be relaxed later on if at all possible.
 *
 */

#include "gl_common.hpp"

#include <fstream>

void
joinNVSwapBarrier(std::ostream &log, GLWindow const *win, uint32_t const group, uint32_t const barrier)
{
	log << "Win32Window::joinNVSwapBarrier : with group = "
		<< group << " and barrier = " << barrier << "." << std::endl;

	if( group == 0 && barrier == 0 )
	{
		log << "Not joining to a swap barrier, because requested "
			<< "group and barrier is 0." << std::endl;
		return; 
	}

	if(!WGL_NV_swap_group)
	{
		log << "NV Swap group extension not supported" << std::endl;
		return;
	}

	uint32_t maxBarrier = 0;
	uint32_t maxGroup = 0;
	wglQueryMaxSwapGroupsNV(win->getDC(), &maxGroup, &maxBarrier);

	if( group > maxGroup )
	{
		log << "Failed to initialize WGL_NV_swap_group: requested group "
				<< group << " greater than maxGroups (" << maxGroup << ")"
				<< std::endl;
		return;
	}

	if( barrier > maxBarrier )
	{
		log << "Failed to initialize WGL_NV_swap_group: requested barrier "
				<< barrier << "greater than maxBarriers (" << maxBarrier << ")"
				<< std::endl;
		return;
	}

	if(!wglJoinSwapGroupNV(win->getDC(), group))
	{
		log << "Failed to join swap group " << group << std::endl;
		return;
	}

	if(wglBindSwapBarrierNV(group, barrier))
	{
		log << "Failed to bind swap barrier " << barrier << std::endl;
		return;
	}

	log << "Joined swap group " << group << " and barrier " << barrier
			<< std::endl;
}

void
leaveNVSwapBarrier(GLWindow const *win, uint32_t const group)
{
	if( group == 0 )
	{ return; }

	// Don't use wglew as if this is the first window, we won't have initialised yet
	// TODO this needs a workaround, creating a fake window for initialisation for example

	// We should be able to use GLEW here
	wglBindSwapBarrierNV( group, 0 );
	wglJoinSwapGroupNV(win->getDC(), 0 );
}

int WINAPI WinMain(	HINSTANCE	hInstance,			// Instance
					HINSTANCE	hPrevInstance,		// Previous Instance
					LPSTR		lpCmdLine,			// Command Line Parameters
					int			nCmdShow)			// Window Show State
{	
	// Create Our OpenGL Window
	GLWindow *win = GLWindow::create("OpenGL swap sync test", 640, 480, 16);

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		MessageBox(NULL, "GLEW init failed.", "ERROR", MB_OK|MB_ICONEXCLAMATION); 
		return 0;
	}

	if(!WGLEW_NV_swap_group)
	{
		MessageBox(NULL, "This demo needs NVidia swap group extension.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
		return 0;
	}

	std::ofstream log_file("debug.log");

	log_file << "Starting NVidia Swap sync demo." << std::endl;

	joinNVSwapBarrier(log_file, win, 1, 1);

	bool done = !win;
	while(!done)
	{
		MSG msg;
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))	// Is There A Message Waiting?
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
					glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear The Screen And The Depth Buffer
					DrawGLScene();					// Draw The Scene
					log_file << "SwapBuffers" << std::endl;
					win->swapBuffers();
					log_file << "Buffers swapped" << std::endl;
				}
			}
		}
	}

	leaveNVSwapBarrier(win, 1);

	// Shutdown
	delete win;
	return 0;
}
