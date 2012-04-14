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

// Interface
#include "OgreWin32GLSupport.h"

#include <OGRE/OgreRoot.h>
#include <OGRE/OgreException.h>
#include <OGRE/OgreLogManager.h>
#include <OGRE/OgreStringConverter.h>

#include <algorithm>

#include "OgreGLTexture.h"
#include "OgreWin32Window.h"

#include <GL/wglew.h>

using namespace Ogre;


/// ----------------------------------- Global -------------------------------
std::string Ogre::translateWGLError(void)
{
	int winError = GetLastError();
	char* errDesc;
	int i;

	errDesc = new char[255];
	// Try windows errors first
	i = FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		winError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) errDesc,
		255,
		NULL
		);

	return String(errDesc);
}

template<class C> void remove_duplicates(C& c)
{
	std::sort(c.begin(), c.end());
	typename C::iterator p = std::unique(c.begin(), c.end());
	c.erase(p, c.end());
}

/// -------------------------- Win32GLSupport --------------------------------
Ogre::Win32GLSupport::Win32GLSupport(void)
{
	// immediately test WGL_ARB_pixel_format and FSAA support
	// so we can set configuration options appropriately
	_initialiseWGL();
}


void
Ogre::Win32GLSupport::addConfig()
{
	//TODO: EnumDisplayDevices http://msdn.microsoft.com/en-us/library/dd162609
	/*vector<string> DisplayDevices;
	DISPLAY_DEVICE DisplayDevice;
	DisplayDevice.cb = sizeof(DISPLAY_DEVICE);
	DWORD i=0;
	while (EnumDisplayDevices(NULL, i++, &DisplayDevice, 0) {
		DisplayDevices.push_back(DisplayDevice.DeviceName);
	}*/
		  
	ConfigOption optFullScreen;
	ConfigOption optVideoMode;
	ConfigOption optColourDepth;
	ConfigOption optDisplayFrequency;
	ConfigOption optVSync;
	ConfigOption optVSyncInterval;
	ConfigOption optFSAA;
	ConfigOption optRTTMode;
	ConfigOption optSRGB;
#ifdef RTSHADER_SYSTEM_BUILD_CORE_SHADERS
	ConfigOption optEnableFixedPipeline;
#endif

	// FS setting possibilities
	optFullScreen.name = "Full Screen";
	optFullScreen.possibleValues.push_back("Yes");
	optFullScreen.possibleValues.push_back("No");
	optFullScreen.currentValue = "Yes";
	optFullScreen.immutable = false;

	// Video mode possibilities
	DEVMODE DevMode;
	DevMode.dmSize = sizeof(DEVMODE);
	optVideoMode.name = "Video Mode";
	optVideoMode.immutable = false;
	for (DWORD i = 0; EnumDisplaySettings(NULL, i, &DevMode); ++i)
	{
		if (DevMode.dmBitsPerPel < 16 || DevMode.dmPelsHeight < 480)
			continue;
		mDevModes.push_back(DevMode);
		StringUtil::StrStreamType str;
		str << DevMode.dmPelsWidth << " x " << DevMode.dmPelsHeight;
		optVideoMode.possibleValues.push_back(str.str());
	}
	remove_duplicates(optVideoMode.possibleValues);
	optVideoMode.currentValue = optVideoMode.possibleValues.front();

	optColourDepth.name = "Colour Depth";
	optColourDepth.immutable = false;
	optColourDepth.currentValue.clear();

	optDisplayFrequency.name = "Display Frequency";
	optDisplayFrequency.immutable = false;
    optDisplayFrequency.currentValue.clear();

	optVSync.name = "VSync";
	optVSync.immutable = false;
	optVSync.possibleValues.push_back("No");
	optVSync.possibleValues.push_back("Yes");
	optVSync.currentValue = "No";

	optVSyncInterval.name = "VSync Interval";
	optVSyncInterval.immutable = false;
	optVSyncInterval.possibleValues.push_back( "1" );
	optVSyncInterval.possibleValues.push_back( "2" );
	optVSyncInterval.possibleValues.push_back( "3" );
	optVSyncInterval.possibleValues.push_back( "4" );
	optVSyncInterval.currentValue = "1";

	optFSAA.name = "FSAA";
	optFSAA.immutable = false;
	optFSAA.possibleValues.push_back("0");
	for (vector<int>::type::iterator it = mFSAALevels.begin(); it != mFSAALevels.end(); ++it)
	{
		String val = StringConverter::toString(*it);
		optFSAA.possibleValues.push_back(val);
		/* not implementing CSAA in GL for now
		if (*it >= 8)
			optFSAA.possibleValues.push_back(val + " [Quality]");
		*/

	}
	optFSAA.currentValue = "0";

	optRTTMode.name = "RTT Preferred Mode";
	optRTTMode.possibleValues.push_back("FBO");
	optRTTMode.currentValue = "FBO";
	optRTTMode.immutable = false;


	// SRGB on auto window
	optSRGB.name = "sRGB Gamma Conversion";
	optSRGB.possibleValues.push_back("Yes");
	optSRGB.possibleValues.push_back("No");
	optSRGB.currentValue = "No";
	optSRGB.immutable = false;

#ifdef RTSHADER_SYSTEM_BUILD_CORE_SHADERS
	optEnableFixedPipeline.name = "Fixed Pipeline Enabled";
	optEnableFixedPipeline.possibleValues.push_back( "Yes" );
	optEnableFixedPipeline.possibleValues.push_back( "No" );
	optEnableFixedPipeline.currentValue = "Yes";
	optEnableFixedPipeline.immutable = false;
#endif

	mOptions[optFullScreen.name] = optFullScreen;
	mOptions[optVideoMode.name] = optVideoMode;
	mOptions[optColourDepth.name] = optColourDepth;
	mOptions[optDisplayFrequency.name] = optDisplayFrequency;
	mOptions[optVSync.name] = optVSync;
	mOptions[optVSyncInterval.name] = optVSyncInterval;
	mOptions[optFSAA.name] = optFSAA;
	mOptions[optRTTMode.name] = optRTTMode;
	mOptions[optSRGB.name] = optSRGB;
#ifdef RTSHADER_SYSTEM_BUILD_CORE_SHADERS
	mOptions[optEnableFixedPipeline.name] = optEnableFixedPipeline;
#endif

	_refreshConfig();
}

void
Ogre::Win32GLSupport::_refreshConfig()
{
	ConfigOptionMap::iterator optVideoMode = mOptions.find("Video Mode");
	ConfigOptionMap::iterator moptColourDepth = mOptions.find("Colour Depth");
	ConfigOptionMap::iterator moptDisplayFrequency = mOptions.find("Display Frequency");
	if(optVideoMode == mOptions.end() || moptColourDepth == mOptions.end() || moptDisplayFrequency == mOptions.end())
		OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Can't find mOptions!", "Win32GLSupport::refreshConfig");
	ConfigOption* optColourDepth = &moptColourDepth->second;
	ConfigOption* optDisplayFrequency = &moptDisplayFrequency->second;

	const String& val = optVideoMode->second.currentValue;
	String::size_type pos = val.find('x');
	if (pos == String::npos)
		OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Invalid Video Mode provided", "Win32GLSupport::refreshConfig");
	DWORD width = StringConverter::parseUnsignedInt(val.substr(0, pos));
	DWORD height = StringConverter::parseUnsignedInt(val.substr(pos+1, String::npos));

	for(vector<DEVMODE>::type::const_iterator i = mDevModes.begin(); i != mDevModes.end(); ++i)
	{
		if (i->dmPelsWidth != width || i->dmPelsHeight != height)
			continue;
		optColourDepth->possibleValues.push_back(StringConverter::toString((unsigned int)i->dmBitsPerPel));
		optDisplayFrequency->possibleValues.push_back(StringConverter::toString((unsigned int)i->dmDisplayFrequency));
	}
	remove_duplicates(optColourDepth->possibleValues);
	remove_duplicates(optDisplayFrequency->possibleValues);
	optColourDepth->currentValue = optColourDepth->possibleValues.back();
	bool freqValid = std::find(optDisplayFrequency->possibleValues.begin(),
		optDisplayFrequency->possibleValues.end(),
		optDisplayFrequency->currentValue) != optDisplayFrequency->possibleValues.end();

	if ( (optDisplayFrequency->currentValue != "N/A") && !freqValid )			
		optDisplayFrequency->currentValue = optDisplayFrequency->possibleValues.front();
}

void
Ogre::Win32GLSupport::setConfigOption(const String &name, const String &value)
{
	ConfigOptionMap::iterator it = mOptions.find(name);

	// Update
	if(it != mOptions.end())
		it->second.currentValue = value;
	else
	{
        StringUtil::StrStreamType str;
        str << "Option named '" << name << "' does not exist.";
		OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, str.str(), "Win32GLSupport::setConfigOption" );
	}

	if( name == "Video Mode" )
		_refreshConfig();

	if( name == "Full Screen" )
	{
		it = mOptions.find( "Display Frequency" );
		if( value == "No" )
		{
			it->second.currentValue = "N/A";
			it->second.immutable = true;
		}
		else
		{
			if (it->second.currentValue.empty() || it->second.currentValue == "N/A")
				it->second.currentValue = it->second.possibleValues.front();
			it->second.immutable = false;
		}
	}
}

Ogre::String
Ogre::Win32GLSupport::validateConfig()
{
	// TODO, DX9
	return StringUtil::BLANK;
}

void
Ogre::Win32GLSupport::start()
{
	LogManager::getSingleton().logMessage("*** Starting Win32GL Subsystem ***");
}

void
Ogre::Win32GLSupport::stop()
{
	LogManager::getSingleton().logMessage("*** Stopping Win32GL Subsystem ***");
}

void
Ogre::Win32GLSupport::_initialiseExtensions(void)
{
	// First, initialise the normal extensions
	GLSupport::_initialiseExtensions();

	// Just copy the extension list from the one we saved from dummy window
	for(set<String>::type::iterator iter = _wgl_extension_list.begin(); 
		iter != _wgl_extension_list.end(); ++iter)
	{
		extensionList.insert(*iter);
	}
}

void
Ogre::Win32GLSupport::_initialiseWGL(void)
{
	std::clog << "Win32GLSupport::initialiseWGL" << std::endl;

	LPCSTR dummyText = "OgreWglDummy";
	// @todo this is idotic I have replaced the modelu name from at least
	// four files already. Can't we make it a const.
#ifdef OGRE_STATIC_LIB
	HINSTANCE hinst = GetModuleHandle( NULL );
#else
#  if OGRE_DEBUG_MODE == 1
	HINSTANCE hinst = GetModuleHandle("HydraGL_d.dll");
#  else
	HINSTANCE hinst = GetModuleHandle("HydraGL.dll");
#  endif
#endif
		
	WNDCLASS dummyClass;
	memset(&dummyClass, 0, sizeof(WNDCLASS));
	dummyClass.style = CS_OWNDC;
	dummyClass.hInstance = hinst;
	dummyClass.lpfnWndProc = _dummyWndProc;
	dummyClass.lpszClassName = dummyText;
	RegisterClass(&dummyClass);

	HWND hwnd = CreateWindow(dummyText, dummyText,
		WS_POPUP | WS_CLIPCHILDREN,
		0, 0, 32, 32, 0, 0, hinst, 0);

	// if a simple CreateWindow fails, then boy are we in trouble...
	if (hwnd == NULL)
	{
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
			"CreateWindow() failed", "Win32GLSupport::initializeWGL");
	}


	// no chance of failure and no need to release thanks to CS_OWNDC
	HDC hdc = GetDC(hwnd); 

	// assign a simple OpenGL pixel format that everyone supports
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.cColorBits = 16;
	pfd.cDepthBits = 15;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
		
	// if these fail, wglCreateContext will also quietly fail
	int format = ChoosePixelFormat(hdc, &pfd);
	if(format != 0)
		SetPixelFormat(hdc, format, &pfd);

	HGLRC hrc = wglCreateContext(hdc);
	if (hrc)
	{
		HGLRC oldrc = wglGetCurrentContext();
		HDC oldhdc = wglGetCurrentDC();

		if(!wglMakeCurrent(hdc, hrc))
		{
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"Failed to make the Dummy window OpenGL context current.", 
				"Win32GLSupport::_initialiseWGL");
		}

		/// initialise GLEW the reason we have this function in the first place
		GLenum err = glewInit();
		if(GLEW_OK != err)
		{
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"Failed to Initialise GLEW.", "Win32GLSupport::_initialiseWGL");
		}

		if (WGLEW_ARB_pixel_format && WGLEW_ARB_multisample)
		{
			// enumerate all formats w/ multisampling
			static const int iattr[] = {
				WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
				WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
				WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
				WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
				WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
                /* We are no matter about the colour, depth and stencil buffers here
				WGL_COLOR_BITS_ARB, 24,
				WGL_ALPHA_BITS_ARB, 8,
				WGL_DEPTH_BITS_ARB, 24,
				WGL_STENCIL_BITS_ARB, 8,
                */
				WGL_SAMPLES_ARB, 2,
				0
			};
			int formats[256];
			unsigned int count;

            if(WGLEW_ARB_pixel_format)
			{
				if(wglChoosePixelFormatARB(hdc, iattr, 0, 256, formats, &count))
				{
					// determine what multisampling levels are offered
					int query = WGL_SAMPLES_ARB, samples;
					for (unsigned int i = 0; i < count; ++i)
					{
						if(wglGetPixelFormatAttribivARB(hdc, formats[i], 0, 1, &query, &samples))
						{
							mFSAALevels.push_back(samples);
						}
					}
					remove_duplicates(mFSAALevels);
				}
			}
		}

		// Get OpenGL extensions we need
		if(!hasFBO())
		{
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"FBOs are not supported.", "Win32GLSupport::_initialiseWGL");
		}
		
		/// Retrieve WGL extensions
		if(WGLEW_ARB_extensions_string)
		{
			const char *wgl_extensions = wglGetExtensionsStringARB(hdc);
			// Parse them, and add them to the wgl extension list
			StringStream ext;
			String instr;
			ext << wgl_extensions;
			while(ext >> instr)
			{
				_wgl_extension_list.insert(instr);
			}
		}

		// Setup GLSupport this needs a valid OpenGL context but it doesn't
		// care if the context is the one we are going to use as
		// extensions can only change if the system changes.
		// @todo this needs to be here because we need valid OpenGL context for
		// this.
		_initialiseExtensions();

		::wglMakeCurrent(oldhdc, oldrc);
		::wglDeleteContext(hrc);
	}

	// clean up our dummy window and class
	DestroyWindow(hwnd);
	UnregisterClass(dummyText, hinst);

	std::clog << "Win32GLSupport::initialiseWGL : successful" << std::endl;
}

bool
Ogre::Win32GLSupport::selectPixelFormat(HDC hdc, GLSupport::PixelFormatOptions const &opt)
{
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = (opt.colourDepth > 16)? 24 : opt.colourDepth;
	pfd.cAlphaBits = (opt.colourDepth > 16)? 8 : 0;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 8;

	if(opt.stereo)
		pfd.dwFlags |= PFD_STEREO;

	int format = 0;

	int useHwGamma = opt.hwGamma? GL_TRUE : GL_FALSE;

	if(opt.multisample && (!WGLEW_ARB_multisample || !WGLEW_ARB_pixel_format))
		return false;

	if(opt.hwGamma && !WGLEW_EXT_framebuffer_sRGB)
		return false;

	// Removing legacy functionality so we use the ARB function if available
	// if it works as desired the fallback to Win32 API call will be removed.
	if(WGLEW_ARB_pixel_format)
	{
		// Use WGL to test extended caps (multisample, sRGB)
		vector<int>::type attribList;
		attribList.push_back(WGL_DRAW_TO_WINDOW_ARB); attribList.push_back(GL_TRUE);
		attribList.push_back(WGL_SUPPORT_OPENGL_ARB); attribList.push_back(GL_TRUE);
		attribList.push_back(WGL_DOUBLE_BUFFER_ARB); attribList.push_back(GL_TRUE);
		attribList.push_back(WGL_SAMPLE_BUFFERS_ARB); attribList.push_back(GL_TRUE);
		attribList.push_back(WGL_ACCELERATION_ARB); attribList.push_back(WGL_FULL_ACCELERATION_ARB);
		attribList.push_back(WGL_COLOR_BITS_ARB); attribList.push_back(pfd.cColorBits);
		attribList.push_back(WGL_ALPHA_BITS_ARB); attribList.push_back(pfd.cAlphaBits);
		attribList.push_back(WGL_DEPTH_BITS_ARB); attribList.push_back(24);
		attribList.push_back(WGL_STENCIL_BITS_ARB); attribList.push_back(8);
		attribList.push_back(WGL_SAMPLES_ARB); attribList.push_back(opt.multisample);
		// hardware gamma
		if (useHwGamma && WGLEW_EXT_framebuffer_sRGB)
		{
			attribList.push_back(WGL_FRAMEBUFFER_SRGB_CAPABLE_EXT); attribList.push_back(GL_TRUE);
		}
		// stereo
		if(opt.stereo)
		{
			attribList.push_back(WGL_STEREO_ARB); attribList.push_back(GL_TRUE);
		}
		// terminator
		attribList.push_back(0);

		UINT nformats;

		if(!wglChoosePixelFormatARB(hdc, &attribList[0], NULL, 1, &format, &nformats) || nformats == 0)
			return false;
	}
	else
	{
		format = ChoosePixelFormat(hdc, &pfd);
	}


	return (format && SetPixelFormat(hdc, format, &pfd));
}

bool
Ogre::Win32GLSupport::selectClosestPixelFormat(HDC dc, GLSupport::PixelFormatOptions const &opt, GLSupport::PixelFormatOptions &real_opts)
{
	real_opts = opt;

	/// Does not try without Gamma correction if user requested it
	/// Because it will affect all the resources also.
	bool formatOk = selectPixelFormat(dc, real_opts);
	if (!formatOk)
	{
		// First try without stereo because it's most likely missing
		if( !formatOk && opt.stereo )
		{
			real_opts.stereo = false;
			formatOk = selectPixelFormat(dc, real_opts);
		}
		
		// Try with stereo but without FSAA
		if( !formatOk && opt.multisample > 0 )
		{
			real_opts.stereo = true;
			real_opts.multisample = 0;
			formatOk = selectPixelFormat(dc, real_opts);
		}

		// Try without stereo and withoutFSAA
		if( !formatOk && opt.multisample > 0 && opt.stereo )
		{
			real_opts.multisample = 0;
			real_opts.stereo = false;
			formatOk = selectPixelFormat(dc, real_opts);
		}
	}

	return formatOk;
}

bool
Ogre::Win32GLSupport::hasFBO(void) const
{
	return (GLEW_VERSION_3_0 || GLEW_EXT_framebuffer_object || GLEW_ARB_framebuffer_object);
}

unsigned int
Ogre::Win32GLSupport::getDisplayMonitorCount() const
{
	if (mMonitorInfoList.empty())		
		EnumDisplayMonitors(NULL, NULL, _sCreateMonitorsInfoEnumProc, (LPARAM)&mMonitorInfoList);

	return (unsigned int)mMonitorInfoList.size();
}

BOOL CALLBACK
Ogre::Win32GLSupport::_sCreateMonitorsInfoEnumProc(
	HMONITOR hMonitor,  // handle to display monitor
	HDC hdcMonitor,     // handle to monitor DC
	LPRECT lprcMonitor, // monitor intersection rectangle
	LPARAM dwData       // data
	)
{
	DisplayMonitorInfoList* pArrMonitorsInfo = (DisplayMonitorInfoList*)dwData;

	// Get monitor info
	DisplayMonitorInfo displayMonitorInfo;

	displayMonitorInfo.hMonitor = hMonitor;

	memset(&displayMonitorInfo.monitorInfoEx, 0, sizeof(MONITORINFOEX));
	displayMonitorInfo.monitorInfoEx.cbSize = sizeof(MONITORINFOEX);
	GetMonitorInfo(hMonitor, &displayMonitorInfo.monitorInfoEx);

	pArrMonitorsInfo->push_back(displayMonitorInfo);

	return TRUE;
}

LRESULT
Ogre::Win32GLSupport::_dummyWndProc(HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp)
{
	return DefWindowProc(hwnd, umsg, wp, lp);
}
