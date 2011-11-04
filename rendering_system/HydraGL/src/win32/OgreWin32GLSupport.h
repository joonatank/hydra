#ifndef __OgreWin32GLSupport_H__
#define __OgreWin32GLSupport_H__

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

		// Virtual override from Ogre::GLSupport
		virtual RenderWindow* createWindow(bool autoCreateWindow, GLRenderSystem* renderSystem, const String& windowTitle = "OGRE Render Window");
		
		/// @copydoc RenderSystem::_createRenderWindow
		// Virtual override from Ogre::GLSupport
		virtual RenderWindow* newWindow(const String &name, unsigned int width, unsigned int height, 
			bool fullScreen, const NameValuePairList *miscParams = 0);

		
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

		/**
		* Get the address of a function
		* Virtual override from Ogre::GLSupport
		*/
		void* getProcAddress(const String& procname);

		/**
		 * Initialise extensions
		 * Virtual override from Ogre::GLSupport
		 */
		virtual void initialiseExtensions();
		
		// Virtual override from Ogre::GLSupport
		virtual unsigned int getDisplayMonitorCount() const;

		// Virtual override from Ogre::GLSupport
		bool selectPixelFormat(HDC hdc, int colourDepth, int multisample, bool hwGamma, bool stereo);

	private:
		// Allowed video modes
		vector<DEVMODE>::type mDevModes;
		Win32Window *mInitialWindow;
		vector<int>::type mFSAALevels;
		bool mHasPixelFormatARB;
        bool mHasMultisample;
		bool mHasHardwareGamma;

		bool mHasNvSwapGroup;
		bool mHasStereo;

		struct DisplayMonitorInfo
		{
			HMONITOR		hMonitor;
			MONITORINFOEX	monitorInfoEx;
		};

		typedef vector<DisplayMonitorInfo>::type DisplayMonitorInfoList;
		typedef DisplayMonitorInfoList::iterator DisplayMonitorInfoIterator;

		DisplayMonitorInfoList mMonitorInfoList;

		void refreshConfig();
		void initialiseWGL();
		static LRESULT CALLBACK dummyWndProc(HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp);
		static BOOL CALLBACK sCreateMonitorsInfoEnumProc(HMONITOR hMonitor, HDC hdcMonitor, 
			LPRECT lprcMonitor, LPARAM dwData);
	};

}

#endif
