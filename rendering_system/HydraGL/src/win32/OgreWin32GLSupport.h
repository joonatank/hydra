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

#ifndef __OgreWin32GLSupport_H__
#define __OgreWin32GLSupport_H__

// Necessary for Windows.h
#include "OgreWin32Prerequisites.h"

#include "OgreGLSupport.h"
#include "OgreGLRenderSystem.h"

namespace Ogre
{

/// ---------------------------------- Global --------------------------------
// Useful error method implemented in Win32GLSupport
String translateWGLError();

/// ---------------------------- Win32GLSupport ------------------------------
class _OgreGLExport Win32GLSupport : public GLSupport
{
public:
	/** @brief Constructor initialises WGL
		*	Creates a valid dummy context and initialises GLEW so we can query
		*	the available functions.
		*/
	Win32GLSupport();

	/**
	* Add any special config values to the system.
	* Must have a "Full Screen" value that is a bool and a "Video Mode" value
	* that is a string in the form of wxhxb
	* Virtual override from Ogre::GLSupport
	*/
	void addConfig();

	// Virtual override from Ogre::GLSupport
	void setConfigOption(const String &name, const String &value);

	/**
	* Make sure all the extra options are valid
	* Virtual override from Ogre::GLSupport
	*/
	String validateConfig();
		
	/**
	* Start anything special
	* Virtual override from Ogre::GLSupport
	*/
	void start();
	/**
	* Stop anything special
	* Virtual override from Ogre::GLSupport
	*/
	void stop();

	// Virtual override from Ogre::GLSupport
	// @todo Where is this used? or what is its primary use case?
	virtual unsigned int getDisplayMonitorCount() const;

	bool selectPixelFormat(HDC hdc, int colourDepth, int multisample, bool hwGamma, bool stereo);

	// Virtual override from Ogre::GLSupport
	bool hasFBO(void) const
	{ return mHasFBO; }

// Private Typedefs
private:

	struct DisplayMonitorInfo
	{
		HMONITOR		hMonitor;
		MONITORINFOEX	monitorInfoEx;
	};

	typedef vector<DisplayMonitorInfo>::type DisplayMonitorInfoList;
	typedef DisplayMonitorInfoList::iterator DisplayMonitorInfoIterator;

/// Private data
private :
	// Allowed video modes
	vector<DEVMODE>::type mDevModes;
	vector<int>::type mFSAALevels;
	bool mHasPixelFormatARB;
    bool mHasMultisample;
	bool mHasHardwareGamma;

	bool mHasNvSwapGroup;
	bool mHasStereo;
	bool mHasFBO;

	// WGL extensions available, retrieved with the dummy window
	set<String>::type _wgl_extension_list;

	DisplayMonitorInfoList mMonitorInfoList;

/// Private methods
private :
	void _refreshConfig(void);
		
	// Create a dummy window and destroy it for providing a valid
	// GLContext for GLEW
	// Using native window creation because the Ogre's Window system is so screwed up
	// you can never be sure which functions call back to the RenderSystem and
	// set up some global state variables...
	void _initialiseWGL(void);

	/**
		* Initialise extensions
		* Virtual override from Ogre::GLSupport
		*/
	void _initialiseExtensions(void);

	static LRESULT CALLBACK _dummyWndProc(HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp);
	static BOOL CALLBACK _sCreateMonitorsInfoEnumProc(HMONITOR hMonitor, HDC hdcMonitor, 
		LPRECT lprcMonitor, LPARAM dwData);

};	// class Win32GLSupport

}	// namespace Ogre

#endif	// __OgreWin32GLSupport_H__
