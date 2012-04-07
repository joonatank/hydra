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

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

// Interface
#include "OgreWin32Window.h"

#include <OGRE/OgreRoot.h>
#include <OGRE/OgreLogManager.h>
#include <OGRE/OgreRenderSystem.h>
#include <OGRE/OgreImageCodec.h>
#include <OGRE/OgreStringConverter.h>
#include <OGRE/OgreException.h>
#include <OGRE/OgreWindowEventUtilities.h>
#include <OGRE/OgreDepthBuffer.h>

#include "OgreWin32GLSupport.h"
#include "OgreWin32Context.h"
#include "OgreGLPixelFormat.h"

#define _MAX_CLASS_NAME_ 128

Ogre::Win32Window::Win32Window(Ogre::Win32GLSupport &glsupport):
	mGLSupport(glsupport),
	mContext(0)
{
	mIsFullScreen = false;
	mHWnd = 0;
	mGlrc = 0;
	mIsExternal = false;
	mIsExternalGLControl = false;
	mIsExternalGLContext = false;
	mSizing = false;
	mClosed = false;
	mHidden = false;
	mVSync = false;
	mVSyncInterval = 1;
	mDisplayFrequency = 0;
	mActive = false;
	mDeviceName = NULL;
	mWindowedWinStyle = 0;
	mFullscreenWinStyle = 0;
}

Ogre::Win32Window::~Win32Window()
{
	destroy();
}

void
Ogre::Win32Window::Options::parse(Ogre::NameValuePairList const &miscParams)
{
	// Get variable-length params
	NameValuePairList::const_iterator opt;
	NameValuePairList::const_iterator end = miscParams.end();

	if ((opt = miscParams.find("title")) != end)
		title = opt->second;

	if ((opt = miscParams.find("left")) != end)
		left = StringConverter::parseInt(opt->second);

	if ((opt = miscParams.find("top")) != end)
		top = StringConverter::parseInt(opt->second);

	if ((opt = miscParams.find("depthBuffer")) != end)
	{
// Don't understand this one
//		mDepthBufferPoolId = StringConverter::parseBool(opt->second) ?
//										DepthBuffer::POOL_DEFAULT : DepthBuffer::POOL_NO_DEPTH;
	}

	if ((opt = miscParams.find("vsync")) != end)
		vsync = StringConverter::parseBool(opt->second);

	if ((opt = miscParams.find("hidden")) != end)
		hidden = StringConverter::parseBool(opt->second);

	if ((opt = miscParams.find("vsyncInterval")) != end)
		vsync_interval = StringConverter::parseUnsignedInt(opt->second);

	if ((opt = miscParams.find("FSAA")) != end)
		fsaa = StringConverter::parseUnsignedInt(opt->second);

	if ((opt = miscParams.find("FSAAHint")) != end)
		fsaa_hint = opt->second;

	if ((opt = miscParams.find("gamma")) != end)
		hwGamma = StringConverter::parseBool(opt->second);

	if ((opt = miscParams.find("externalWindowHandle")) != end)
	{
#if OGRE_ARCH_TYPE == OGRE_ARCHITECTURE_64
		std::istringstream ss( opt->second );
		uint64 pointer;
		ss >> std::hex >> pointer;
		hwnd = (HWND)pointer;
#else
 		hwnd = (HWND)StringConverter::parseUnsignedInt(opt->second);
#endif
		if(hwnd)
		{
			external_window = true;
			fullscreen = false;
		}

		if ((opt = miscParams.find("externalGLControl")) != end)
		{
			externalGLControl = StringConverter::parseBool(opt->second);
		}
	}
	if ((opt = miscParams.find("externalGLContext")) != end)
	{
#if OGRE_ARCH_TYPE == OGRE_ARCHITECTURE_64
		std::istringstream ss( opt->second );
		uint64 pointer;
		ss >> std::hex >> pointer;
		glrc = (HGLRC)pointer;
#else
		glrc = (HGLRC)StringConverter::parseUnsignedInt(opt->second);
#endif
		if(glrc)
			externalGLContext = true;
	}

	// window border style
	opt = miscParams.find("border");
	if(opt != miscParams.end())
		border = opt->second;
	// set outer dimensions?
	opt = miscParams.find("outerDimensions");
	if(opt != miscParams.end())
		outerSize = StringConverter::parseBool(opt->second);

	// only available with fullscreen
	if ((opt = miscParams.find("displayFrequency")) != end)
		frequency = StringConverter::parseUnsignedInt(opt->second);
	if ((opt = miscParams.find("colourDepth")) != end)
	{
		colour_depth = StringConverter::parseUnsignedInt(opt->second);
		if (!fullscreen)
		{
			// make sure we don't exceed desktop colour depth
			if ((int)colour_depth > GetDeviceCaps(GetDC(0), BITSPIXEL))
				colour_depth = GetDeviceCaps(GetDC(0), BITSPIXEL);
		}
	}

	// incompatible with fullscreen
	if ((opt = miscParams.find("parentWindowHandle")) != end)
	{
#if OGRE_ARCH_TYPE == OGRE_ARCHITECTURE_64
		std::istringstream ss( opt->second );
		uint64 pointer;
		ss >> std::hex >> pointer;
		parent = (HWND)pointer;
#else
 		parent = (HWND)StringConverter::parseUnsignedInt(opt->second);
#endif
	}


	// monitor index
	if ((opt = miscParams.find("monitorIndex")) != end)
		monitorIndex = StringConverter::parseInt(opt->second);
			
	// monitor handle
	if ((opt = miscParams.find("monitorHandle")) != end)
		hMonitor = (HMONITOR)StringConverter::parseInt(opt->second);

	// enable double click messages
	if ((opt = miscParams.find("enableDoubleClick")) != end)
		enableDoubleClick = StringConverter::parseBool(opt->second);

	if ((opt = miscParams.find("stereo")) != end)
		stereo = StringConverter::parseBool(opt->second);
}

void
Ogre::Win32Window::create(const String& name, unsigned int width, unsigned int height,
						bool fullScreen, const Ogre::NameValuePairList *miscParams)
{
	// destroy current window, if any
	if (mHWnd)
		destroy();

#ifdef OGRE_STATIC_LIB
	HINSTANCE hInst = GetModuleHandle( NULL );
#else
#  if OGRE_DEBUG_MODE == 1
	HINSTANCE hInst = GetModuleHandle("HydraGL_D.dll");
#  else
	HINSTANCE hInst = GetModuleHandle("HydraGL.dll");
#  endif
#endif

	// Set members
	mHWnd = 0;
	mName = name;
	mClosed = false;
	mDepthBufferPoolId = DepthBuffer::POOL_DEFAULT;
	
	// Options
	Options options;
	options.fullscreen = fullScreen;
	options.title = name;
	options.frequency = 0;
	options.colour_depth = mIsFullScreen? 32 : GetDeviceCaps(GetDC(0), BITSPIXEL);

	// Parsing misc parameters should be in separate function
	if(miscParams)
	{ options.parse(*miscParams); }

	if(!options.external_window)
	{
		DWORD		  dwStyleEx = 0;					
		MONITORINFOEX monitorInfoEx;
		RECT		  rc;
			
		// If we didn't specified the adapter index, or if it didn't find it
		if(options.hMonitor == NULL)
		{
			POINT windowAnchorPoint;

			// Fill in anchor point.
			windowAnchorPoint.x = options.left;
			windowAnchorPoint.y = options.top;


			// Get the nearest monitor to this window.
			options.hMonitor = MonitorFromPoint(windowAnchorPoint, MONITOR_DEFAULTTONEAREST);
		}

		// Get the target monitor info		
		memset(&monitorInfoEx, 0, sizeof(MONITORINFOEX));
		monitorInfoEx.cbSize = sizeof(MONITORINFOEX);
		GetMonitorInfo(options.hMonitor, &monitorInfoEx);

		size_t devNameLen = strlen(monitorInfoEx.szDevice);
		mDeviceName = new char[devNameLen + 1];

		strcpy(mDeviceName, monitorInfoEx.szDevice);

		// Update window style flags.
		mFullscreenWinStyle = (options.hidden ? 0 : WS_VISIBLE) | WS_CLIPCHILDREN | WS_POPUP;
		mWindowedWinStyle   = (options.hidden ? 0 : WS_VISIBLE) | WS_CLIPCHILDREN;
			
		if(options.parent)
		{
			mWindowedWinStyle |= WS_CHILD;
		}
		else
		{
			if (options.border == "none")
			{ mWindowedWinStyle |= WS_POPUP; }
			else if (options.border == "fixed")
			{ mWindowedWinStyle |= WS_OVERLAPPED | WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX; }
			else
			{ mWindowedWinStyle |= WS_OVERLAPPEDWINDOW; }
		}


		// No specified top left -> Center the window in the middle of the monitor
		if (options.left == -1 || options.top == -1)
		{				
			int screenw = monitorInfoEx.rcWork.right  - monitorInfoEx.rcWork.left;
			int screenh = monitorInfoEx.rcWork.bottom - monitorInfoEx.rcWork.top;

			unsigned int winWidth, winHeight;
			adjustWindow(width, height, &winWidth, &winHeight);

			// clamp window dimensions to screen size
			int outerw = (winWidth < screenw)? winWidth : screenw;
			int outerh = (winHeight < screenh)? winHeight : screenh;

			if (options.left == -1)
				options.left = monitorInfoEx.rcWork.left + (screenw - outerw) / 2;
			else if (options.monitorIndex != -1)
				options.left += monitorInfoEx.rcWork.left;

			if (options.top == -1)
				options.top = monitorInfoEx.rcWork.top + (screenh - outerh) / 2;
			else if (options.monitorIndex != -1)
				options.top += monitorInfoEx.rcWork.top;
		}
		else if (options.monitorIndex != -1)
		{
			options.left += monitorInfoEx.rcWork.left;
			options.top += monitorInfoEx.rcWork.top;
		}

		mWidth = width;
		mHeight = height;
		mTop = options.top;
		mLeft = options.left;

		if(options.fullscreen)
		{				
			dwStyleEx |= WS_EX_TOPMOST;
			mTop = monitorInfoEx.rcMonitor.top;
			mLeft = monitorInfoEx.rcMonitor.left;											
		}
		else
		{
			int screenw = GetSystemMetrics(SM_CXSCREEN);
			int screenh = GetSystemMetrics(SM_CYSCREEN);

			if (!options.outerSize)
			{
				// Calculate window dimensions required
				// to get the requested client area
				SetRect(&rc, 0, 0, mWidth, mHeight);
				AdjustWindowRect(&rc, getWindowStyle(fullScreen), false);
				mWidth = rc.right - rc.left;
				mHeight = rc.bottom - rc.top;

				// Clamp window rect to the nearest display monitor.
				if (mLeft < monitorInfoEx.rcWork.left)
					mLeft = monitorInfoEx.rcWork.left;		

				if (mTop < monitorInfoEx.rcWork.top)					
					mTop = monitorInfoEx.rcWork.top;					

				if ((int)mWidth > monitorInfoEx.rcWork.right - mLeft)					
					mWidth = monitorInfoEx.rcWork.right - mLeft;	

				if ((int)mHeight > monitorInfoEx.rcWork.bottom - mTop)					
					mHeight = monitorInfoEx.rcWork.bottom - mTop;		
			}			
		}
 
		UINT classStyle = CS_OWNDC;
		if (options.enableDoubleClick)
		{ classStyle |= CS_DBLCLKS; }

		// register class and create window
		WNDCLASS wc = { classStyle, WindowEventUtilities::_WndProc, 0, 0, hInst,
			LoadIcon(NULL, IDI_APPLICATION), LoadCursor(NULL, IDC_ARROW),
			(HBRUSH)GetStockObject(BLACK_BRUSH), NULL, "OgreGLWindow" };
		RegisterClass(&wc);

		// Pass pointer to self as WM_CREATE parameter
		mHWnd = CreateWindowEx(dwStyleEx, "OgreGLWindow", options.title.c_str(),
			getWindowStyle(options.fullscreen), mLeft, mTop, mWidth, mHeight, 
			options.parent, 0, hInst, this);

		WindowEventUtilities::_addRenderWindow(this);

		LogManager::getSingleton().stream() << "Created Win32Window '" 
			<< mName << "' : " << mWidth << "x" << mHeight << ", " 
			<< options.colour_depth << "bpp";
	}

	// @todo can't we just run setFullscreen() ?
	// @todo No idea if this works but it's so much cleaner that
	// if we need it we are going to use this.
	if(options.fullscreen)
	{ setFullscreen(options.fullscreen, mWidth, mHeight); }

	RECT rc;
	// top and left represent outer window position
	GetWindowRect(mHWnd, &rc);
	mTop = rc.top;
	mLeft = rc.left;
	// width and height represent drawable area only
	GetClientRect(mHWnd, &rc);
	mWidth = rc.right;
	mHeight = rc.bottom;

	mHDC = GetDC(mHWnd);

	GLSupport::PixelFormatOptions format_opt;
	format_opt.stereo = options.stereo;
	format_opt.hwGamma = options.hwGamma;
	format_opt.colourDepth = options.colour_depth;
	format_opt.multisample = options.fsaa;
	
	_initialise_gl(format_opt);

	setHidden(options.hidden);
}

void
Ogre::Win32Window::_initialise_gl(GLSupport::PixelFormatOptions const &opt)
{
	HDC old_hdc = wglGetCurrentDC();
	HGLRC old_context = wglGetCurrentContext();

	if(!mIsExternalGLControl)
	{
		if(!_selectPixelFormat(mHDC, opt))
		{
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"selectPixelFormat failed", "Win32Window::create");
		}
	}

	if(!mIsExternalGLContext)
	{
		const HDC dc = mHDC;
		mGlrc = wglCreateContext(dc);
		if (!mGlrc)
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
			"wglCreateContext failed: " + translateWGLError(), "Win32Window::create");
	}
	if (!wglMakeCurrent(mHDC, mGlrc))
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "wglMakeCurrent", "Win32Window::create");

	// Do not change vsync if the external window has the OpenGL control
	if(!mIsExternalGLControl) 
	{
		if (wglSwapIntervalEXT)
		{ wglSwapIntervalEXT(mVSync? mVSyncInterval : 0); }
	}

    if(old_context && old_context != mGlrc)
    {
		std::clog << "Has an old context" << std::endl;
        // Restore old context
		if (!wglMakeCurrent(old_hdc, old_context))
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "wglMakeCurrent() failed", "Win32Window::create");

        // Share lists with old context
		if (!wglShareLists(old_context, mGlrc))
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "wglShareLists() failed", " Win32Window::create");
    }

	// Create RenderSystem context
	mContext = new Win32Context(mHDC, mGlrc);

	mActive = true;
}

void
Ogre::Win32Window::adjustWindow(unsigned int clientWidth, unsigned int clientHeight, 
	unsigned int* winWidth, unsigned int* winHeight)
{
	// NB only call this for non full screen
	RECT rc;
	SetRect(&rc, 0, 0, clientWidth, clientHeight);
	AdjustWindowRect(&rc, getWindowStyle(mIsFullScreen), false);
	*winWidth = rc.right - rc.left;
	*winHeight = rc.bottom - rc.top;

	// adjust to monitor
	HMONITOR hMonitor = MonitorFromWindow(mHWnd, MONITOR_DEFAULTTONEAREST);

	// Get monitor info	
	MONITORINFO monitorInfo;

	memset(&monitorInfo, 0, sizeof(MONITORINFO));
	monitorInfo.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(hMonitor, &monitorInfo);

	LONG maxW = monitorInfo.rcWork.right  - monitorInfo.rcWork.left;
	LONG maxH = monitorInfo.rcWork.bottom - monitorInfo.rcWork.top;

	if (*winWidth > (unsigned int)maxW)
		*winWidth = maxW;
	if (*winHeight > (unsigned int)maxH)
		*winHeight = maxH;

}

void
Ogre::Win32Window::setFullscreen(bool fullScreen, unsigned int width, unsigned int height)
{
	if (mIsFullScreen != fullScreen || width != mWidth || height != mHeight)
	{
		mIsFullScreen = fullScreen;
			
		if (mIsFullScreen)
		{	
			DEVMODE displayDeviceMode;

			memset(&displayDeviceMode, 0, sizeof(displayDeviceMode));
			displayDeviceMode.dmSize = sizeof(DEVMODE);
			displayDeviceMode.dmBitsPerPel = mColourDepth;
			displayDeviceMode.dmPelsWidth = width;
			displayDeviceMode.dmPelsHeight = height;
			displayDeviceMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
			if (mDisplayFrequency)
			{
				displayDeviceMode.dmDisplayFrequency = mDisplayFrequency;
				displayDeviceMode.dmFields |= DM_DISPLAYFREQUENCY;

				if (ChangeDisplaySettingsEx(mDeviceName, &displayDeviceMode, NULL, 
					CDS_FULLSCREEN | CDS_TEST, NULL) != DISP_CHANGE_SUCCESSFUL)					
				{
					LogManager::getSingleton().logMessage(LML_NORMAL, "ChangeDisplaySettings with user display frequency failed");
					displayDeviceMode.dmFields ^= DM_DISPLAYFREQUENCY;
				}
			}
			else
			{
				// try a few
				displayDeviceMode.dmDisplayFrequency = 100;
				displayDeviceMode.dmFields |= DM_DISPLAYFREQUENCY;
				if (ChangeDisplaySettingsEx(mDeviceName, &displayDeviceMode, NULL, 
					CDS_FULLSCREEN | CDS_TEST, NULL) != DISP_CHANGE_SUCCESSFUL)		
				{
					displayDeviceMode.dmDisplayFrequency = 75;
					if (ChangeDisplaySettingsEx(mDeviceName, &displayDeviceMode, NULL, 
						CDS_FULLSCREEN | CDS_TEST, NULL) != DISP_CHANGE_SUCCESSFUL)		
					{
						displayDeviceMode.dmFields ^= DM_DISPLAYFREQUENCY;
					}
				}

			}
			// move window to 0,0 before display switch
			SetWindowPos(mHWnd, HWND_TOPMOST, 0, 0, mWidth, mHeight, SWP_NOACTIVATE);

			if (ChangeDisplaySettingsEx(mDeviceName, &displayDeviceMode, NULL, CDS_FULLSCREEN, NULL) != DISP_CHANGE_SUCCESSFUL)				
				LogManager::getSingleton().logMessage(LML_CRITICAL, "ChangeDisplaySettings failed");

			// Get the nearest monitor to this window.
			HMONITOR hMonitor = MonitorFromWindow(mHWnd, MONITOR_DEFAULTTONEAREST);

			// Get monitor info	
			MONITORINFO monitorInfo;

			memset(&monitorInfo, 0, sizeof(MONITORINFO));
			monitorInfo.cbSize = sizeof(MONITORINFO);
			GetMonitorInfo(hMonitor, &monitorInfo);

			mTop = monitorInfo.rcMonitor.top;
			mLeft = monitorInfo.rcMonitor.left;

			SetWindowLong(mHWnd, GWL_STYLE, getWindowStyle(mIsFullScreen));
			SetWindowPos(mHWnd, HWND_TOPMOST, mLeft, mTop, width, height,
				SWP_NOACTIVATE);
			mWidth = width;
			mHeight = height;
		}
		else
		{				
			// drop out of fullscreen
			ChangeDisplaySettingsEx(mDeviceName, NULL, NULL, 0, NULL);

			// calculate overall dimensions for requested client area
			unsigned int winWidth, winHeight;
			adjustWindow(width, height, &winWidth, &winHeight);

			// deal with centering when switching down to smaller resolution

			HMONITOR hMonitor = MonitorFromWindow(mHWnd, MONITOR_DEFAULTTONEAREST);
			MONITORINFO monitorInfo;
			memset(&monitorInfo, 0, sizeof(MONITORINFO));
			monitorInfo.cbSize = sizeof(MONITORINFO);
			GetMonitorInfo(hMonitor, &monitorInfo);

			LONG screenw = monitorInfo.rcWork.right  - monitorInfo.rcWork.left;
			LONG screenh = monitorInfo.rcWork.bottom - monitorInfo.rcWork.top;


			int left = screenw > winWidth ? ((screenw - winWidth) / 2) : 0;
			int top = screenh > winHeight ? ((screenh - winHeight) / 2) : 0;

			SetWindowLong(mHWnd, GWL_STYLE, getWindowStyle(mIsFullScreen));
			SetWindowPos(mHWnd, HWND_NOTOPMOST, left, top, winWidth, winHeight,
				SWP_DRAWFRAME | SWP_FRAMECHANGED | SWP_NOACTIVATE);
			mWidth = width;
			mHeight = height;

			windowMovedOrResized();

		}

	}
}

void
Ogre::Win32Window::destroy(void)
{
	if (!mHWnd)
		return;

	// Unregister and destroy OGRE GLContext
	delete mContext;

	if (!mIsExternalGLContext && mGlrc)
	{
		wglDeleteContext(mGlrc);
		mGlrc = 0;
	}
	if (!mIsExternal)
	{
		WindowEventUtilities::_removeRenderWindow(this);

		if (mIsFullScreen)
			ChangeDisplaySettingsEx(mDeviceName, NULL, NULL, 0, NULL);
		DestroyWindow(mHWnd);
	}
	else
	{
		// just release the DC
		ReleaseDC(mHWnd, mHDC);
	}

	mActive = false;
	mClosed = true;
	mHDC = 0; // no release thanks to CS_OWNDC wndclass style
	mHWnd = 0;

	if (mDeviceName != NULL)
	{
		delete[] mDeviceName;
		mDeviceName = NULL;
	}
		
}


bool
Ogre::Win32Window::isActive(void) const
{
	if (isFullScreen())
		return isVisible();

	return mActive && isVisible();
}

bool
Ogre::Win32Window::isVisible() const
{
	return (mHWnd && !IsIconic(mHWnd));
}

bool
Ogre::Win32Window::isClosed() const
{
	return mClosed;
}

void
Ogre::Win32Window::setHidden(bool hidden)
{
	mHidden = hidden;
	if (!mIsExternal)
	{
		if (hidden)
			ShowWindow(mHWnd, SW_HIDE);
		else
			ShowWindow(mHWnd, SW_SHOWNORMAL);
	}
}

void
Ogre::Win32Window::setVSyncEnabled(bool vsync)
{
	mVSync = vsync;
	HDC old_hdc = wglGetCurrentDC();
	HGLRC old_context = wglGetCurrentContext();
	if (!wglMakeCurrent(mHDC, mGlrc))
	{
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "wglMakeCurrent",
			"Win32Window::setVSyncEnabled");
	}

	// Do not change vsync if the external window has the OpenGL control
	if (!mIsExternalGLControl)
	{
		if(WGLEW_EXT_swap_control)
		{
			if(!wglSwapIntervalEXT(mVSync ? mVSyncInterval : 0))
			{ std::clog << "wglSwapIntervalEXT : FAILED" << std::endl; }
		}
	}

    if (old_context && old_context != mGlrc)
    {
        // Restore old context
		if (!wglMakeCurrent(old_hdc, old_context))
		{
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"wglMakeCurrent() failed", "Win32Window::setVSyncEnabled");
		}
	}
}

void
Ogre::Win32Window::setVSyncInterval(unsigned int interval)
{
	mVSyncInterval = interval;
	if (mVSync)
		setVSyncEnabled(true);
}

bool
Ogre::Win32Window::isVSyncEnabled() const
{
	return mVSync;
}

unsigned int
Ogre::Win32Window::getVSyncInterval() const
{
	return mVSyncInterval;
}

void
Ogre::Win32Window::reposition(int left, int top)
{
	if (mHWnd && !mIsFullScreen)
	{
		SetWindowPos(mHWnd, 0, left, top, 0, 0,
			SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
	}
}

void
Ogre::Win32Window::resize(unsigned int width, unsigned int height)
{
	if (mHWnd && !mIsFullScreen)
	{
		RECT rc = { 0, 0, width, height };
		AdjustWindowRect(&rc, getWindowStyle(mIsFullScreen), false);
		width = rc.right - rc.left;
		height = rc.bottom - rc.top;
		SetWindowPos(mHWnd, 0, 0, 0, width, height,
			SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	}
}

void
Ogre::Win32Window::windowMovedOrResized(void)
{
	if (!mHWnd || IsIconic(mHWnd))
		return;

	updateWindowRect();		
}


void
Ogre::Win32Window::updateWindowRect(void)
{
	RECT rc;
	BOOL result;

	// Update top left parameters
	result = GetWindowRect(mHWnd, &rc);
	if (result == FALSE)
	{
		mTop = 0;
		mLeft = 0;
		mWidth = 0;
		mHeight = 0;
		return;
	}

	mTop = rc.top;
	mLeft = rc.left;

	// width and height represent drawable area only
	result = GetClientRect(mHWnd, &rc);
	if (result == FALSE)
	{
		mTop = 0;
		mLeft = 0;
		mWidth = 0;
		mHeight = 0;
		return;
	}
	unsigned int width = rc.right - rc.left;
	unsigned int height = rc.bottom - rc.top;

	// Case window resized.
	if (width != mWidth || height != mHeight)
	{
		mWidth  = rc.right - rc.left;
		mHeight = rc.bottom - rc.top;

		// Notify viewports of resize
		ViewportList::iterator it = mViewportList.begin();
		while( it != mViewportList.end() )
			(*it++).second->_updateDimensions();			
	}
}


void
Ogre::Win32Window::swapBuffers(bool waitForVSync)
{
	if (!mIsExternalGLControl)
	{
		SwapBuffers(mHDC);
	}
}

void
Ogre::Win32Window::copyContentsToMemory(const PixelBox &dst, FrameBuffer buffer)
{
	/// @todo this is slow as hell
	/// after moving all rendering to be into FBOs we should replace this with a
	/// copy of the FBO to a texture file
	if ((dst.left < 0) || (dst.right > mWidth) ||
		(dst.top < 0) || (dst.bottom > mHeight) ||
		(dst.front != 0) || (dst.back != 1))
	{
		OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
					"Invalid box.", "Win32Window::copyContentsToMemory" );
	}

	if (buffer == FB_AUTO)
	{
		buffer = mIsFullScreen? FB_FRONT : FB_BACK;
	}

	GLenum format = Ogre::GLPixelUtil::getGLOriginFormat(dst.format);
	GLenum type = Ogre::GLPixelUtil::getGLOriginDataType(dst.format);

	if ((format == GL_NONE) || (type == 0))
	{
		OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
					"Unsupported format.", "Win32Window::copyContentsToMemory" );
	}


	// Switch context if different from current one
	RenderSystem* rsys = Root::getSingleton().getRenderSystem();
	rsys->_setViewport(this->getViewport(0));

	// Must change the packing to ensure no overruns!
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	glReadBuffer((buffer == FB_FRONT)? GL_FRONT : GL_BACK);
	glReadPixels((GLint)dst.left, (GLint)dst.top,
					(GLsizei)dst.getWidth(), (GLsizei)dst.getHeight(),
					format, type, dst.data);

	// restore default alignment
	glPixelStorei(GL_PACK_ALIGNMENT, 4);

	//vertical flip
	{
		size_t rowSpan = dst.getWidth() * PixelUtil::getNumElemBytes(dst.format);
		size_t height = dst.getHeight();
		uchar *tmpData = new uchar[rowSpan * height];
		uchar *srcRow = (uchar *)dst.data, *tmpRow = tmpData + (height - 1) * rowSpan;

		while (tmpRow >= tmpData)
		{
			memcpy(tmpRow, srcRow, rowSpan);
			srcRow += rowSpan;
			tmpRow -= rowSpan;
		}
		memcpy(dst.data, tmpData, rowSpan * height);

		delete [] tmpData;
	}
}

void
Ogre::Win32Window::getCustomAttribute( const String& name, void* pData )
{
	if( name == "GLCONTEXT" )
	{
		*static_cast<GLContext**>(pData) = mContext;
		return;
	}
	else if( name == "WINDOW" )
	{
		HWND *pHwnd = (HWND*)pData;
		*pHwnd = getWindowHandle();
		return;
	}
}

void
Ogre::Win32Window::setActive(bool state)
{	
	if (mDeviceName != NULL && state == false)
	{
		HWND hActiveWindow = GetActiveWindow();
		char classNameSrc[_MAX_CLASS_NAME_ + 1];
		char classNameDst[_MAX_CLASS_NAME_ + 1];

		GetClassName(mHWnd, classNameSrc, _MAX_CLASS_NAME_);
		GetClassName(hActiveWindow, classNameDst, _MAX_CLASS_NAME_);

		if (strcmp(classNameDst, classNameSrc) == 0)
		{
			state = true;
		}						
	}
		
	mActive = state;

	if( mIsFullScreen )
	{
		if( state == false )
		{	//Restore Desktop
			ChangeDisplaySettingsEx(mDeviceName, NULL, NULL, 0, NULL);
			ShowWindow(mHWnd, SW_SHOWMINNOACTIVE);
		}
		else
		{	//Restore App
			ShowWindow(mHWnd, SW_SHOWNORMAL);

			DEVMODE displayDeviceMode;

			memset(&displayDeviceMode, 0, sizeof(displayDeviceMode));
			displayDeviceMode.dmSize = sizeof(DEVMODE);
			displayDeviceMode.dmBitsPerPel = mColourDepth;
			displayDeviceMode.dmPelsWidth = mWidth;
			displayDeviceMode.dmPelsHeight = mHeight;
			displayDeviceMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
			if (mDisplayFrequency)
			{
				displayDeviceMode.dmDisplayFrequency = mDisplayFrequency;
				displayDeviceMode.dmFields |= DM_DISPLAYFREQUENCY;
			}
			ChangeDisplaySettingsEx(mDeviceName, &displayDeviceMode, NULL, CDS_FULLSCREEN, NULL);
		}
	}
}

bool
Ogre::Win32Window::_selectPixelFormat(HDC dc, GLSupport::PixelFormatOptions const &opt)
{
	GLSupport::PixelFormatOptions real_opts;
	bool retval = mGLSupport.selectClosestPixelFormat(dc, opt, real_opts);

	// record what gamma option we used in the end
	// this will control enabling of sRGB state flags when used
	mHwGamma = real_opts.hwGamma;
	mFSAA = real_opts.multisample;

	return retval;
}
