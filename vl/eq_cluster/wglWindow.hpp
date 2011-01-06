/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-12
 *
 */

#ifndef EQOGRE_WGLWINDOW_HPP
#define EQOGRE_WGLWINDOW_HPP

#include <eq/client/wglWindow.h>

namespace eqOgre
{

class WGLWindow : public eq::WGLWindowIF
{
public :
	WGLWindow( eq::Window *parent );

	virtual ~WGLWindow( void );

	/** @return the WGL rendering context. @version 1.0 */
    virtual HGLRC getWGLContext() const { return _wglContext; }
	/** @return the Win32 window handle. @version 1.0 */
	virtual HWND getWGLWindowHandle() const
	{ return _wglWindow; }

	/** @return the Win32 off screen PBuffer handle. @version 1.0 */
	virtual HPBUFFERARB getWGLPBufferHandle() const
	{ return _wglPBuffer; }

	/**
	 * @return the Win32 device context used for the current drawable.
	 * @version 1.0
	 */
	virtual HDC getWGLDC() const 
	{ return _wglDC; }

	/** @return the Win32 affinity device context, if used. @version 1.0 */
	//virtual HDC getWGLAffinityDC();

	/** @name WGL/Win32 initialization */
	//@{
	/** 
	 * Initialize this window for the WGL window system.
	 *
	 * This method first calls initWGLAffinityDC, then chooses a pixel
	 * format with chooseWGLPixelFormat, then creates a drawable using 
	 * configInitWGLDrawable and finally creates the context using
	 * createWGLContext.
	 * 
	 * @return true if the initialization was successful, false otherwise.
	 * @version 1.0
	 */
	virtual bool configInit();

	/** @version 1.0 */
	virtual void configExit( );

	/** 
	 * Initialize the window's drawable (pbuffer or window) and
	 * bind the WGL context.
	 *
	 * Sets the window handle on success.
	 * 
	 * @param pixelFormat the window's target pixel format.
	 * @return true if the drawable was created, false otherwise.
	 * @version 1.0
	 */
	virtual bool configInitWGLDrawable( int pixelFormat );

	/** 
	 * Initialize the window with an on-screen Win32 window.
	 *
	 * Sets the window handle on success.
	 * 
	 * @param pixelFormat the window's target pixel format.
	 * @return true if the drawable was created, false otherwise.
	 * @version 1.0
	 */
	virtual bool configInitWGLWindow( int pixelFormat );

	/** @name Operations. */
	//@{
	/** @version 1.0 */
	virtual void makeCurrent() const;

	/** @version 1.0 */
	virtual void swapBuffers();

	/** Join the WGL_NV_swap_group. @version 1.0 */
	// Need to be overriden because pure virtual
	virtual void joinNVSwapBarrier( const uint32_t group,
									const uint32_t barrier )
	{}


	/** Unbind a WGL_NV_swap_barrier. @version 1.0 */ 
//	void leaveNVSwapBarrier();

	/// Event handling
	/** Process the given event. @version 1.0 */
	// Needs to be overiden, pure virtual in parent
	virtual bool processEvent( const eq::WGLWindowEvent& event );

private :
	/** 
	 * Create a WGL context.
	 * 
	 * This method does not set the window's WGL context.
	 *
	 * @return the context, or 0 if context creation failed.
	 * @version 1.0
	 */
	virtual HGLRC createWGLContext();

	virtual void setWGLWindowHandle( HWND handle );

	/** Destroy the given WGL context. @version 1.0 */
	virtual void destroyWGLContext( HGLRC context );

	virtual int chooseWGLPixelFormat();

	/** Create an unmapped WGL window. */
	HWND _createWGLWindow( int pixelFormat, const eq::PixelViewport& pvp );

	/** Use wglChoosePixelFormatARB */
	int _chooseWGLPixelFormatARB( HDC pfDC );

	/** Use ChoosePixelFormat */
	int _chooseWGLPixelFormat( HDC pfDC );

	HWND             _wglWindow;
	HPBUFFERARB      _wglPBuffer;
	HGLRC            _wglContext;

	HDC              _wglDC;
//	HDC              _wglAffinityDC;

	BOOL             _screenSaverActive;

	uint32_t         _wglNVSwapGroup;

};	// class WGLWindow

}	// namespace eqOgre

#endif // EQOGRE_WGLWINDOW_HPP
