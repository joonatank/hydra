/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

#ifndef __Win32Window_H__
#define __Win32Window_H__

#include "OgreWin32Prerequisites.h"

#include <OGRE/OgreRenderWindow.h>

#include <stdint.h>

namespace Ogre {

class _OgreGLExport Win32Window : public RenderWindow
{
public:
	Win32Window(Win32GLSupport &glsupport);
	~Win32Window();

	/// Virtual override, have a look at Ogre::RenderWindow
	void create(const String& name, unsigned int width, unsigned int height,
		bool fullScreen, const NameValuePairList *miscParams);
	/// Virtual override, have a look at Ogre::RenderWindow
	void setFullscreen(bool fullScreen, unsigned int width, unsigned int height);
	/// Virtual override, have a look at Ogre::RenderWindow
	void destroy(void);
	
	// Method for dealing with resize / move & 3d library
	/// Virtual override, have a look at Ogre::RenderWindow
	virtual void windowMovedOrResized(void);
	/// Virtual override, have a look at Ogre::RenderWindow
	void reposition(int left, int top);
	/// Virtual override, have a look at Ogre::RenderWindow
	void resize(unsigned int width, unsigned int height);

	/// Virtual override, have a look at Ogre::RenderWindow
	bool isVisible() const;
	/// Virtual override, have a look at Ogre::RenderWindow
	bool isHidden() const { return mHidden; }
	/// Virtual override, have a look at Ogre::RenderWindow
	void setHidden(bool hidden);

	/// Virtual override, have a look at Ogre::RenderWindow
	void setVSyncEnabled(bool vsync);
	/// Virtual override, have a look at Ogre::RenderWindow
	bool isVSyncEnabled() const;
	/// Virtual override, have a look at Ogre::RenderWindow
	void setVSyncInterval(unsigned int interval);
	/// Virtual override, have a look at Ogre::RenderWindow
	unsigned int getVSyncInterval() const;

	/// Virtual override, have a look at Ogre::RenderWindow
	bool isActive(void) const;
	/// Virtual override, have a look at Ogre::RenderWindow
	bool isClosed(void) const;

	/// Virtual override, have a look at Ogre::RenderTarget
	virtual void copyContentsToMemory(const PixelBox &dst, FrameBuffer buffer);

	/// Virtual override, have a look at Ogre::RenderTarget
	void swapBuffers(bool waitForVSync);

	/// Virtual override, have a look at Ogre::RenderTarget
	bool requiresTextureFlipping() const { return false; }

	/// Virtual override, have a look at Ogre::RenderTarget
	void getCustomAttribute(std::string const &name, void* pData);

    /// @brief Used to set the active state of the render target.
	/// Virtual override, have a look at Ogre::RenderTarget
    virtual void setActive( bool state );


	/// Not virtual overrides
	void adjustWindow(unsigned int clientWidth, unsigned int clientHeight, 
		unsigned int* winWidth, unsigned int* winHeight);

	HWND getWindowHandle(void) const { return mHWnd; }
	HDC getHDC(void) const { return mHDC; }

protected:
		
	/** Update the window rect. */ 
	void updateWindowRect(void);

	/** Return the target window style depending on the fullscreen parameter. */
	DWORD getWindowStyle(bool fullScreen) const { if (fullScreen) return mFullscreenWinStyle; return mWindowedWinStyle; }

	bool selectPixelFormat(HDC dc, bool hwGamma, bool stereo);

protected:
	Win32GLSupport &mGLSupport;

	// Windows handles
	// @todo shouldn't these be wrapped inside a GLContext object?
	// would allow us to do proper clean up automatically.
	HWND	mHWnd;	// window handle
	HDC		mHDC;	// device
	HGLRC	mGlrc;	// GL context
    
	// We own both a context wrapper and a native handle, what is up with this?
	Win32Context *mContext;

	bool    mIsExternal;
	char*   mDeviceName;
	bool    mIsExternalGLControl;
	bool	mIsExternalGLContext;
    bool    mSizing;
	bool	mClosed;
	bool	mHidden;
	bool	mVSync;
	unsigned int mVSyncInterval;
    int     mDisplayFrequency;      // fullscreen only, to restore display
	
	DWORD	mWindowedWinStyle;		// Windowed mode window style flags.
	DWORD	mFullscreenWinStyle;	// Fullscreen mode window style flags.

};	// class Win32Window

}	// namespace Ogre

#endif	// __Win32Window_H__
