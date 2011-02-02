/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2011-01
 *
 *	Abstract interface for OpenGL rendering window
 */

#ifndef HYDRA_GL_WINDOW_HPP
#define HYDRA_GL_WINDOW_HPP

namespace eqOgre
{

class GLWindow
{
public :
	GLWindow( void ) {}

	virtual ~GLWindow( void ) {}

	virtual void makeCurrent( void ) const = 0;
};

}

#endif	// HYDRA_GL_WINDOW_HPP