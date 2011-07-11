/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-06
 *	@file gl_common.hpp
 *
 *	This file is part of Hydra a VR game engine tests.
 *
 *	Common functions for the usage of OpenGL, window creation and so on
 *	used by the minimal OpenGL test programs.
 *
 *	Works only on Windows.
 */

#ifndef HYDRA_GL_COMMON_HPP
#define HYDRA_GL_COMMON_HPP

#include <GL/glew.h>

#ifdef _WIN32
#include <windows.h>		// Header File For Windows
#include <GL/wglew.h>
#endif

#include <gl\gl.h>			// Header File For The OpenGL32 Library
#include <gl\glu.h>			// Header File For The GLu32 Library

#include <iostream>
#include <stdint.h>

/// @brief OpenGL context wrapper
struct GLContext
{
	/// @brief constructor
	/// @param hdc the device context where the OpenGL Context is created
	GLContext(HDC hdc)
		: _device(hdc), _gl_context(0)
	{
		// Are We Able To Get A Rendering Context?
		if( !(_gl_context=wglCreateContext(_device)) )
		{
			// TODO should throw
			throw std::string("Can't Create A GL Rendering Context.");
		}

		// Try To Activate The Rendering Context
		if(!wglMakeCurrent(_device, _gl_context))
		{
			// TODO should throw
			throw std::string("Can't Activate The GL Rendering Context.");
		}
	}

	~GLContext(void)
	{
		if(_gl_context)
		{
			// Are We Able To Release The DC And RC Contexts?
			if( !wglMakeCurrent(NULL,NULL) )
			{
				MessageBox(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
			}

			// Are We Able To Delete The RC?
			if( !wglDeleteContext(_gl_context) )
			{
				MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
			}
		}
	}

	bool makeCurrent(void)
	{
		// @todo should throw
		return wglMakeCurrent(_device, _gl_context);
	}

	HGLRC getGLContext(void)
	{ return _gl_context; }

	HDC getDevice(void)
	{ return _device; }

private :
	HDC _device;

	// Permanent Rendering Context
	HGLRC _gl_context;

};

class GLWindow
{
public :
	static GLWindow *create(char* title, int width, int height, int bits, GLContext *ctx = 0);

	~GLWindow(void);

	void swapBuffers(void)
	{ ::SwapBuffers(hDC); }

	// Get GDI Device Context
	HDC getDC(void) const
	{ return hDC; }

	HWND getHandle(void) const
	{ return hWnd; }

	GLContext *getContext(void)
	{ return _context; }

	void makeCurrent(void)
	{ _context->makeCurrent(); }

private :
	// TODO shouldn't need the HDC
	GLWindow(HDC hdc, HWND win, GLContext *context)
		: hDC(hdc), hWnd(win), _context(context)
	{}

	HDC			hDC;		// Private GDI Device Context
	HWND		hWnd;		// Holds Our Window Handle

	GLContext *_context;
};

GLvoid KillGLWindow(HINSTANCE hInstance, GLWindow &win);


LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

inline GLvoid ReSizeGLScene(GLsizei width, GLsizei height)		// Resize And Initialize The GL Window
{
	if (height==0)										// Prevent A Divide By Zero By
	{
		height=1;										// Making Height Equal One
	}

	glViewport(0,0,width,height);						// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
}

inline int InitGL(GLvoid)										// All Setup For OpenGL Goes Here
{
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations

	return TRUE;										// Initialization Went OK
}

inline int DrawGLScene(GLvoid)									// Here's Where We Do All The Drawing
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();						// Reset The View

	glColor3f(0.0f, 0.0f, 0.0f);
	glTranslatef(-1.5f,0.0f,-6.0f);					// Move Left 1.5 Units And Into The Screen 6.0
	glBegin(GL_TRIANGLES);						// Drawing Using Triangles
		glVertex3f( 0.0f, 1.0f, 0.0f);				// Top
		glVertex3f(-1.0f,-1.0f, 0.0f);				// Bottom Left
		glVertex3f( 1.0f,-1.0f, 0.0f);				// Bottom Right
	glEnd();							// Finished Drawing The Triangle

	glColor3f(1.0f, 1.0f, 0.0f);
	glTranslatef(3.0f,0.0f,0.0f);					// Move Right 3 Units
	glBegin(GL_QUADS);						// Draw A Quad
		glVertex3f(-1.0f, 1.0f, 0.0f);				// Top Left
		glVertex3f( 1.0f, 1.0f, 0.0f);				// Top Right
		glVertex3f( 1.0f,-1.0f, 0.0f);				// Bottom Right
		glVertex3f(-1.0f,-1.0f, 0.0f);				// Bottom Left
	glEnd();							// Done Drawing The Quad

	return TRUE;							// Keep Going
}

inline PIXELFORMATDESCRIPTOR
getPixelFormat(DWORD dwFlags, int bits)
{
	PIXELFORMATDESCRIPTOR pfd=				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		dwFlags,
		PFD_TYPE_RGBA,								// Request An RGBA Format
		bits,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};

	return pfd;
}

/*	This Code Creates Our OpenGL Window.  Parameters Are:					*
 *	title			- Title To Appear At The Top Of The Window				*
 *	width			- Width Of The GL Window Or Fullscreen Mode				*
 *	height			- Height Of The GL Window Or Fullscreen Mode			*
 *	bits			- Number Of Bits To Use For Color (8/16/24/32)			*
 *	fullscreenflag	- Use Fullscreen Mode (TRUE) Or Windowed Mode (FALSE)	*/ 
inline GLWindow *
GLWindow::create(char* title, int width, int height, int bits, GLContext *ctx)
{
	GLuint		PixelFormat;			// Holds The Results After Searching For A Match
	WNDCLASS	wc;						// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left=(long)0;			// Set Left Value To 0
	WindowRect.right=(long)width;		// Set Right Value To Requested Width
	WindowRect.top=(long)0;				// Set Top Value To 0
	WindowRect.bottom=(long)height;		// Set Bottom Value To Requested Height

	HINSTANCE hInstance	= GetModuleHandle(NULL);				// Grab An Instance For Our Window
	
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc		= (WNDPROC) WndProc;					// WndProc Handles Messages
	wc.cbClsExtra		= 0;									// No Extra Window Data
	wc.cbWndExtra		= 0;									// No Extra Window Data
	wc.hInstance		= hInstance;							// Set The Instance
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground	= NULL;									// No Background Required For GL
	wc.lpszMenuName		= NULL;									// We Don't Want A Menu
	wc.lpszClassName	= "OpenGL";								// Set The Class Name

	if (!RegisterClass(&wc))									// Attempt To Register The Window Class
	{
		MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return 0;
	}

	dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
	dwStyle=WS_OVERLAPPEDWINDOW;							// Windows Style

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

	HWND hWnd = 0;
	// Create The Window
	if (!(hWnd=CreateWindowEx(	dwExStyle,							// Extended Style For The Window
								"OpenGL",							// Class Name
								title,								// Window Title
								dwStyle |							// Defined Window Style
								WS_CLIPSIBLINGS |					// Required Window Style
								WS_CLIPCHILDREN,					// Required Window Style
								0, 0,								// Window Position
								WindowRect.right-WindowRect.left,	// Calculate Window Width
								WindowRect.bottom-WindowRect.top,	// Calculate Window Height
								NULL,								// No Parent Window
								NULL,								// No Menu
								hInstance,							// Instance
								NULL)))								// Dont Pass Anything To WM_CREATE
	{
		//KillGLWindow(hInstance, GLWindow(0, 0, hWnd));								// Reset The Display
		// TODO cleanup
		MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return 0;
	}

	std::clog << "Window created." << std::endl;
	
	HDC hDC = 0;
	if(ctx)
	{
		hDC = ctx->getDevice();

		if(!ctx->makeCurrent())
		{
			MessageBox(NULL,"Failed to make provided context current.","ERROR",MB_OK|MB_ICONEXCLAMATION);
			return 0;
		}
	}
	else if( !(hDC=GetDC(hWnd)) )
	{
		//KillGLWindow(hInstance, GLWindow(hDC, 0, hWnd));								// Reset The Display
		// TODO cleanup
		MessageBox(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return 0;
	}

	DWORD dwFlags = PFD_DRAW_TO_WINDOW |	// Format Must Support Window
		PFD_SUPPORT_OPENGL |				// Format Must Support OpenGL
		PFD_DOUBLEBUFFER;					// Must Support Double Buffering

	PIXELFORMATDESCRIPTOR pfd = getPixelFormat(dwFlags | PFD_STEREO, bits);
	if( !(PixelFormat=ChoosePixelFormat(hDC, &pfd)) )
	{
		// Without quad-buffer stereo 
		pfd = getPixelFormat(dwFlags, bits);
		if( !(PixelFormat=ChoosePixelFormat(hDC,&pfd)) )
		{
			//KillGLWindow(hInstance, GLWindow(hDC, 0, 0));
			// TODO cleanup
			MessageBox(NULL,"Can't Find A Suitable PixelFormat.","ERROR", MB_OK|MB_ICONEXCLAMATION);
			return 0;
		}
		else
		{
			std::clog << "Selecting NON stereo pixel format" << std::endl;
		}
	}
	else
	{
		std::clog << "Selecting stereo pixel format" << std::endl;
	}

	if(!SetPixelFormat(hDC, PixelFormat, &pfd))		// Are We Able To Set The Pixel Format?
	{
		//KillGLWindow(hInstance, GLWindow(hDC, 0, hWnd));								// Reset The Display
		// TODO cleanup
		MessageBox(NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return 0;
	}

	GLContext *context = ctx;
	if(!context)
		context = new GLContext(hDC);

	ShowWindow(hWnd, SW_SHOW);						// Show The Window
	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	SetFocus(hWnd);									// Sets Keyboard Focus To The Window
	ReSizeGLScene(width, height);					// Set Up Our Perspective GL Screen

	std::clog << "Initing OpenGL" << std::endl;

	if (!InitGL())									// Initialize Our Newly Created GL Window
	{
		//KillGLWindow(hInstance, GLWindow(hDC, hRC, hWnd));								// Reset The Display
		// TODO cleanup
		MessageBox(NULL,"Initialization Failed.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return 0;
	}

	std::clog << "OpenGL inited" << std::endl;

	return new GLWindow(hDC, hWnd, context);
}


inline
GLWindow::~GLWindow(void)
{
	delete _context;

	if(hDC && !ReleaseDC(hWnd, hDC))					// Are We Able To Release The DC
	{
		MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
	}

	if(hWnd && !DestroyWindow(hWnd))					// Are We Able To Destroy The Window?
	{
		MessageBox(NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
	}

	HINSTANCE hInstance	= GetModuleHandle(NULL);				// Grab An Instance For Our Window
	if(!UnregisterClass("OpenGL", hInstance))			// Are We Able To Unregister Class
	{
		MessageBox(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
	}
}


bool	g_keys[256];			// Array Used For The Keyboard Routine
bool	g_active=TRUE;		// Window Active Flag Set To TRUE By Default

inline LRESULT CALLBACK WndProc(	HWND	hWnd,			// Handle For This Window
							UINT	uMsg,			// Message For This Window
							WPARAM	wParam,			// Additional Message Information
							LPARAM	lParam)			// Additional Message Information
{
	switch (uMsg)									// Check For Windows Messages
	{
		case WM_ACTIVATE:							// Watch For Window Activate Message
		{
			if (!HIWORD(wParam))					// Check Minimization State
			{
				g_active=TRUE;						// Program Is Active
			}
			else
			{
				g_active=FALSE;						// Program Is No Longer Active
			}

			return 0;								// Return To The Message Loop
		}

		case WM_SYSCOMMAND:							// Intercept System Commands
		{
			switch (wParam)							// Check System Calls
			{
				case SC_SCREENSAVE:					// Screensaver Trying To Start?
				case SC_MONITORPOWER:				// Monitor Trying To Enter Powersave?
				return 0;							// Prevent From Happening
			}
			break;									// Exit
		}

		case WM_CLOSE:								// Did We Receive A Close Message?
		{
			PostQuitMessage(0);						// Send A Quit Message
			return 0;								// Jump Back
		}

		case WM_KEYDOWN:							// Is A Key Being Held Down?
		{
			g_keys[wParam] = TRUE;					// If So, Mark It As TRUE
			return 0;								// Jump Back
		}

		case WM_KEYUP:								// Has A Key Been Released?
		{
			g_keys[wParam] = FALSE;					// If So, Mark It As FALSE
			return 0;								// Jump Back
		}

		case WM_SIZE:								// Resize The OpenGL Window
		{
			ReSizeGLScene(LOWORD(lParam),HIWORD(lParam));  // LoWord=Width, HiWord=Height
			return 0;								// Jump Back
		}
	}

	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

#endif // HYDRA_GL_COMMON_HPP
