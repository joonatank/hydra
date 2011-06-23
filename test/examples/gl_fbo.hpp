/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-06
 *	@file gl_fbo.hpp
 *
 *	This file is part of Hydra a VR game engine tests.
 *
 *	Common class for creating an FBO, rendering to it and drawing it into 
 *	the current viewport of the current context.
 *
 *	Works only on Windows.
 */

#ifndef HYDRA_EXAMPLES_GL_FBO_HPP
#define HYDRA_EXAMPLES_GL_FBO_HPP

void drawToScreen(uint32_t width, uint32_t height, GLuint texture)
{
	glEnable(GL_TEXTURE_2D);

	//glViewport(0, 0, width, height);
	// Draw the FBO to the Screen
	glClearColor(0.0f, 1.0f, 0.0f, 0.0f);
	// If we don't clear depth buffer this does not draw the texture to the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, (GLfloat)width, 0, GLfloat(height), -10, 10);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glBindTexture(GL_TEXTURE_2D, texture);
					
	glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2f(0, 0);
		glTexCoord2f(1, 0);
		glVertex2f(width, 0);
		glTexCoord2f(1, 1);
		glVertex2f(width, height);
		glTexCoord2f(0, 1);
		glVertex2f(0, height);
	glEnd();
					
	glDisable(GL_TEXTURE_2D);
}

class FBO
{
public :
	FBO(uint32_t width, uint32_t height)
		: _width(width), _height(height)
	{
		glGenTextures(1, &_fbo_texture);

		//create the colorbuffer texture and attach it to the frame buffer
		glBindTexture(GL_TEXTURE_2D, _fbo_texture);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, _width, _height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	
		if(glGetError() != GL_NO_ERROR)
		{ std::clog << "Error in create FBO texture" << std::endl; }
		glBindTexture(GL_TEXTURE_2D, 0);
				
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
	}

	void endDraw(void)
	{
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
		::drawToScreen(_width, _height, _fbo_texture);
		/*
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
		*/
	}

private :
	uint32_t _width;
	uint32_t _height;

	GLuint _fbo_texture;
	GLuint _fbo;

};	// class FBO

#endif	// HYDRA_EXAMPLES_GL_FBO_HPP
