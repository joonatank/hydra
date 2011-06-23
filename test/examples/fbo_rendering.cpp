
#ifdef _WIN32

#include "gl_common.hpp"

//#include "wglext.h"

#include <fstream>

struct FBO
{
	FBO(uint32_t width, uint32_t height)
		: _width(width), _height(height)
	{
		glGenTextures(1, &_fbo_texture);

		//create the colorbuffer texture and attach it to the frame buffer
		glBindTexture(GL_TEXTURE_2D, _fbo_texture);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		// @todo shouldn't matter if we set the texels or not,
		// but we can test the texture rendering
		unsigned char* texels = new unsigned char[_width * _height * 3];
		// grey texture
		for(size_t i = 0; i < _width*_height*3; ++i)
		{ texels[i] = 127; }
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, _width, _height, 0, GL_RGB, GL_UNSIGNED_BYTE, texels);
	
		if(glGetError() != GL_NO_ERROR)
		{ std::clog << "Error in create FBO texture" << std::endl; }
		glBindTexture(GL_TEXTURE_2D, 0);

		delete [] texels;
		texels = 0;
				
		//switch to our fbo so we can bind stuff to it
		glGenFramebuffers(1, &_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _fbo_texture, 0);
				
		// check FBO status

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if(status != GL_FRAMEBUFFER_COMPLETE)
		{ std::clog << "FBO is incomplete!" << std::endl; }

		// Unbind the frame buffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	~FBO()
	{
		// TODO cleanup
		//glDeleteFramebuffers(GL_FRAMEBUFFER, &_fbo);
	}

	void beginDraw(void)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
//		glPushAttrib(GL_VIEWPORT_BIT);
//		glViewport(0, 0, _width, _height);

		// define render targets (empty texture is at GL_COLOR_ATTACHMENT0)
		// not necessary
		//glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);

//		glClearColor(0.0f, 0.0f, 1.0f, 0.0f);
//		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				
		// Working till this
//		glMatrixMode(GL_PROJECTION);
//		glLoadIdentity();
		//glViewport(0, 0, mWidth, mHeight);
//		glOrtho(-1, 1, -1, 1, 0.01, 20);
//		if(glGetError() != GL_NO_ERROR)
//		{ std::clog << "Error in setting the projection matrix" << std::endl; }
				
//		glMatrixMode(GL_MODELVIEW);
//		glLoadIdentity();
//		glTranslatef(0, 0, -10);

//		glDisable(GL_TEXTURE_2D);

		// setting colour will cause the screen to be black
		//glColor3f(0.0f, 1.0f, 0.0f);
			
		/*
		glBegin(GL_QUADS);
				glVertex3f(-.5f, .5f, 0.0f);				// Top Left
				glVertex3f( .5f, .5f, 0.0f);				// Top Right
				glVertex3f( .5f,-.5f, 0.0f);				// Bottom Right
				glVertex3f(-.5f,-.5f, 0.0f);				// Bottom Left
		glEnd();
		
		glBegin(GL_QUADS);
			glVertex3f(-1.0f, 1.0f, 0.0f);				// Top Left
			glVertex3f( 0.0f, 1.0f, 0.0f);				// Top Right
			glVertex3f( 0.0f,-0.0f, 0.0f);				// Bottom Right
			glVertex3f(-1.0f,-0.0f, 0.0f);				// Bottom Left
		glEnd();
		if(glGetError() != GL_NO_ERROR)
		{ std::clog << "Error in GL draw" << std::endl; }
		*/
	}

	void endDraw(void)
	{
//		glPopAttrib();
		/// Unbind the framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		if(glGetError() != GL_NO_ERROR)
		{ std::clog << "Unbinding the frame buffer" << std::endl; }
	}

	GLuint getTexture(void) const
	{ return _fbo_texture; }

	/// @brief draws the FBO to the current OpenGL context, uses the whole viewport
	void drawToScreen(void)
	{
		glEnable(GL_TEXTURE_2D);

		//glViewport(0, 0, width, height);
		// Draw the FBO to the Screen
		glClearColor(0.0f, 1.0f, 0.0f, 0.0f);
		// If we don't clear depth buffer this does not draw the texture to the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, (GLfloat)_width, 0, GLfloat(_height), -10, 10);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glBindTexture(GL_TEXTURE_2D, _fbo_texture);
					
		glBegin(GL_QUADS);
			glTexCoord2f(0, 0);
			glVertex2f(0, 0);
			glTexCoord2f(1, 0);
			glVertex2f(_width, 0);
			glTexCoord2f(1, 1);
			glVertex2f(_width, _height);
			glTexCoord2f(0, 1);
			glVertex2f(0, _height);
		glEnd();
					
		glDisable(GL_TEXTURE_2D);
	}

	uint32_t _width;
	uint32_t _height;

	GLuint _fbo_texture;
	GLuint _fbo;
};

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

#endif	// ifndef _WIN32