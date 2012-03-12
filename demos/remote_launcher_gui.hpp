/**
 *	Copyright (c) 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2012-03
 *	@file remote_launcher_gui.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Contains Windows specific GUI code for the Launcher
 *
 */

#ifndef REMOTE_LAUNCHER_GUI_HPP
#define REMOTE_LAUNCHER_GUI_HPP

#ifndef _WIN32
#error "Only Windows implementation provided"
#endif

#include "remote_launcher.hpp"

// Windows headers
#include <windows.h>
#include <shellapi.h>

#define ID_TRAY_APP_ICON                5000
#define ID_TRAY_EXIT_CONTEXT_MENU_ITEM  3000
#define WM_TRAYICON ( WM_USER + 1 )

UINT WM_TASKBARCREATED = 0;

LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);


#define     ID_START       1000
#define     ID_INSTALL     1001
#define     ID_EXIT        1002
#define		ID_KILL		   1003

class LauncherWindow
{
public :
	LauncherWindow(RemoteLauncher *launcher, HINSTANCE hInstance, int iCmdShow);

	~LauncherWindow(void);

	LRESULT injectEvent(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	void initNotifyIconData(void);

	void minimise(void);

	void restore(void);

private :
	void _install(void);
	void _start(void);
	void _kill(void);
	void _exit(void);
	void _save_ini(void);
	
	void _set_title(void);

	RemoteLauncher *_launcher;

	HWND _hwnd;
	HMENU _tray_menu;
	HMENU _main_menu;
	NOTIFYICONDATA _notifyIconData;
};

extern LauncherWindow *g_launcher_gui;

LauncherWindow::LauncherWindow(RemoteLauncher *launcher, HINSTANCE hInstance, int iCmdShow)
	: _launcher(launcher)
	, _hwnd(0)
	, _tray_menu(0)
	, _main_menu(0)
{
	TCHAR className[] = TEXT( "HydraRemoteLauncher" );

	// I want to be notified when windows explorer
	// crashes and re-launches the taskbar.  the WM_TASKBARCREATED
	// event will be sent to my WndProc() AUTOMATICALLY whenever
	// explorer.exe starts up and fires up the taskbar again.
	// So its great, because now, even if explorer crashes,
	// I have a way to re-add my system tray icon in case
	// the app is already in the "minimized" (hidden) state.
	// if we did not do this an explorer crashed, the application
	// would remain inaccessible!!
	WM_TASKBARCREATED = RegisterWindowMessageA("TaskbarCreated") ;

	WNDCLASSEX wnd = { 0 };

	wnd.hInstance = hInstance;
	wnd.lpszClassName = className;
	wnd.lpfnWndProc = WndProc;
	wnd.style = CS_HREDRAW | CS_VREDRAW ;
	wnd.cbSize = sizeof (WNDCLASSEX);

	wnd.hIcon = LoadIcon (NULL, IDI_APPLICATION);
	wnd.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
	wnd.hCursor = LoadCursor (NULL, IDC_ARROW);
	wnd.hbrBackground = (HBRUSH)COLOR_APPWORKSPACE ;

	if (!RegisterClassEx(&wnd))
	{
		FatalAppExit( 0, TEXT("Couldn't register window class!") );
	}

	_hwnd = CreateWindowEx(
		0, className,
		TEXT( "HydraRemoteLauncher" ),
		WS_MINIMIZE | WS_BORDER | WS_SYSMENU | WS_CAPTION,
		CW_USEDEFAULT, CW_USEDEFAULT,
		640, 480,
		NULL, NULL,
		hInstance, NULL
	);
	assert(_hwnd);

	_set_title();

	// Add the label with instruction text
	const char *text = "Hydra Remote Launcher\n"
		" When this we are running we will automatically launch slaves on this computer\n"
		" Provided you have correctly configured the remote launcher port, hydra path and slave name\n"
		" You can find the configuration file in AppData/Hydra/hydra_remote_launcher.ini\n"
		" Install adds us to Windows startup.\n"
		" Pressing close will minimise us to system tray.\n"
		" Exit from either tray menu or file menu";
	CreateWindow( TEXT("static"), text, WS_CHILD | WS_VISIBLE,
					0, 0, 400, 400, _hwnd, 0, hInstance, NULL ) ;

	_main_menu = CreateMenu();
    SetMenu(_hwnd, _main_menu);

    HMENU hSubMenu = CreatePopupMenu();
    AppendMenu(hSubMenu, MF_STRING, ID_START, "&Start");
	AppendMenu(hSubMenu, MF_STRING, ID_KILL,  TEXT( "Kill Hydra" ) );
    AppendMenu(hSubMenu, MF_STRING, ID_INSTALL, "&Install");
    AppendMenu(hSubMenu, MF_STRING, ID_EXIT, "E&xit");
    AppendMenu(hSubMenu, MF_SEPARATOR, 0, NULL);
    InsertMenu(_main_menu, 0, MF_POPUP|MF_BYPOSITION, (UINT_PTR)hSubMenu, "File");

    DrawMenuBar(_hwnd);

	// Initialize the NOTIFYICONDATA structure once
	initNotifyIconData();

	// create the menu once.
	// oddly, you don't seem to have to explicitly attach
	// the menu to the HWND at all.  This seems so ODD.
	_tray_menu = CreatePopupMenu();

	AppendMenu(_tray_menu, MF_STRING, ID_START,  TEXT( "Start" ) );
	AppendMenu(_tray_menu, MF_STRING, ID_KILL,  TEXT( "Kill Hydra" ) );
	AppendMenu(_tray_menu, MF_STRING, ID_INSTALL,  TEXT( "Install" ) );
	AppendMenu(_tray_menu, MF_STRING, ID_EXIT,  TEXT( "Exit" ) );

	ShowWindow (_hwnd, iCmdShow);
}

LauncherWindow::~LauncherWindow(void)
{
	// Once you get the quit message, before exiting the app,
	// clean up and remove the tray icon
	if( !IsWindowVisible(_hwnd) )
	{
		Shell_NotifyIcon(NIM_DELETE, &_notifyIconData);
	}
}

void
LauncherWindow::_install(void)
{
	MessageBox(_hwnd, "Should install the Remote Launcher", "Remote Launcher", MB_OK);
	_launcher->install();
}

void
LauncherWindow::_start(void)
{
	assert(_launcher);
	
	_launcher->setRunning(!_launcher->isRunning());

	_set_title();
}

void
LauncherWindow::_set_title(void)
{
	assert(_launcher);

	const char *str = 0;
	if(_launcher->isRunning())
	{
		str = "HydraRemoteLauncher : started";
	}
	else
	{
		str = "HydraRemoteLauncher : stopped";
	}
	
	SetWindowText(_hwnd, str);
	strcpy(_notifyIconData.szTip, TEXT(str));
	Shell_NotifyIcon(NIM_MODIFY, &_notifyIconData);
}

void
LauncherWindow::_kill(void)
{
	MessageBox(_hwnd, "Should kill Hydra Instances", "Remote Launcher", MB_OK);
	_launcher->kill_hydra_instances();
}

void
LauncherWindow::_exit(void)
{
	PostQuitMessage( 0 ) ;
}

void
LauncherWindow::_save_ini(void)
{
	_launcher->getOptions().save_ini();
}


// Initialize the NOTIFYICONDATA structure.
// See MSDN docs http://msdn.microsoft.com/en-us/library/bb773352(VS.85).aspx
// for details on the NOTIFYICONDATA structure.
void
LauncherWindow::initNotifyIconData(void)
{
	memset( &_notifyIconData, 0, sizeof( NOTIFYICONDATA ) ) ;

	_notifyIconData.cbSize = sizeof(NOTIFYICONDATA);

	/////
	// Tie the NOTIFYICONDATA struct to our
	// global HWND (that will have been initialized
	// before calling this function)
	_notifyIconData.hWnd = _hwnd;
	// Now GIVE the NOTIFYICON.. the thing that
	// will sit in the system tray, an ID.
	_notifyIconData.uID = ID_TRAY_APP_ICON;
	// The COMBINATION of HWND and uID form
	// a UNIQUE identifier for EACH ITEM in the
	// system tray.  Windows knows which application
	// each icon in the system tray belongs to
	// by the HWND parameter.
	/////

	/////
	// Set up flags.
	_notifyIconData.uFlags = NIF_ICON | // promise that the hIcon member WILL BE A VALID ICON!!
							 NIF_MESSAGE | // when someone clicks on the system tray icon,
			 				 // we want a WM_ type message to be sent to our WNDPROC
							 NIF_TIP;      // we're gonna provide a tooltip as well, son.

	_notifyIconData.uCallbackMessage = WM_TRAYICON;

	_notifyIconData.hIcon = (HICON)LoadImage( NULL, TEXT("Security.ico"), IMAGE_ICON, 0, 0, LR_LOADFROMFILE  ) ;

	// set the tooltip text.  must be LESS THAN 64 chars
	strcpy(_notifyIconData.szTip, TEXT("HydraRemoteLauncher : stopped"));
}


// These next 2 functions are the STARS of this example.
// They perform the "minimization" function and "restore"
// functions for our window.  Notice how when you "minimize"
// the app, it doesn't really "minimize" at all.  Instead,
// you simply HIDE the window, so it doesn't display, and
// at the same time, stick in a little icon in the system tray,
// so the user can still access the application.
void
LauncherWindow::minimise(void)
{
	// add the icon to the system tray
	Shell_NotifyIcon(NIM_ADD, &_notifyIconData);

	// ..and hide the main window
	ShowWindow(_hwnd, SW_HIDE);
}

// Basically bring back the window (SHOW IT again)
// and remove the little icon in the system tray.
void
LauncherWindow::restore(void)
{
	// Remove the icon from the system tray
	Shell_NotifyIcon(NIM_DELETE, &_notifyIconData);

	// ..and show the window
	ShowWindow(_hwnd, SW_SHOW);
}

LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if(g_launcher_gui)
		return g_launcher_gui->injectEvent(hwnd, message, wParam, lParam);

	return DefWindowProc(hwnd, message, wParam, lParam);
}

LRESULT
LauncherWindow::injectEvent(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if ( message==WM_TASKBARCREATED && !IsWindowVisible(_hwnd) )
	{
		minimise();
		return 0;
	}

	switch (message)
	{
		case WM_CREATE:
		{
		}
		break;

		case WM_SYSCOMMAND:
		{
			// (filter out reserved lower 4 bits:  see msdn remarks http://msdn.microsoft.com/en-us/library/ms646360(VS.85).aspx)
			switch( wParam & 0xfff0 )
			{
				case SC_MINIMIZE:
				case SC_CLOSE:  // redundant to WM_CLOSE, it appears
					minimise() ;
					return 0 ;
					break;
			}
		}
		break;

		case WM_COMMAND :
		{
			switch(LOWORD(wParam))
			{
			case ID_INSTALL :
				this->_install();
				break;
			case ID_START :
				this->_start();
				break;

			case ID_KILL :
				this->_kill();
				break;

			case ID_EXIT :
				this->_exit();
				break;
			}
		}
		break;

		// Our user defined WM_TRAYICON message.
		// We made this message up, and we told
		//
		case WM_TRAYICON:
		{
			printf( "Tray icon notification, from %d\n", wParam ) ;

			switch(wParam)
			{
				case ID_TRAY_APP_ICON:
					printf( "Its the ID_TRAY_APP_ICON.. one app can have several tray icons, ya know..\n" ) ;
				break;
			}

			// the mouse button has been released.

			// I'd LIKE TO do this on WM_LBUTTONDOWN, it makes
			// for a more responsive-feeling app but actually
			// the guy who made the original post is right.
			// Most apps DO respond to WM_LBUTTONUP, so if you
			// restore your window on WM_LBUTTONDOWN, then some
			// other icon will scroll in under your mouse so when
			// the user releases the mouse, THAT OTHER ICON will
			// get the WM_LBUTTONUP command and that's quite annoying.
			if (lParam == WM_LBUTTONUP)
			{
				restore();
			}
			else if (lParam == WM_RBUTTONDOWN) // I'm using WM_RBUTTONDOWN here because
			{
				printf( "Mmm.  Let's get contextual.  I'm showing you my context menu.\n" ) ;
				// it gives the app a more responsive feel.  Some apps
				// DO use this trick as well.  Right clicks won't make
				// the icon disappear, so you don't get any annoying behavior
				// with this (try it out!)

				// Get current mouse position.
				POINT curPoint ;
				GetCursorPos( &curPoint ) ;

				// should SetForegroundWindow according
				// to original poster so the popup shows on top
				SetForegroundWindow(hwnd); 

				// TrackPopupMenu blocks the app until TrackPopupMenu returns
				printf("calling track\n");
				UINT clicked = TrackPopupMenu(
					_tray_menu,
					TPM_RETURNCMD | TPM_NONOTIFY, // don't send me WM_COMMAND messages about this window, instead return the identifier of the clicked menu item
					curPoint.x,
					curPoint.y,
					0,
					hwnd,
					NULL
					);

				printf("returned from call to track\n");

				// Original poster's line of code.  Haven't deleted it,
				// but haven't seen a need for it.
				//SendMessage(hwnd, WM_NULL, 0, 0); // send benign message to window to make sure the menu goes away.
				switch(clicked)
				{
					case ID_INSTALL :
						this->_install();
						break;
					case ID_START :
						this->_start();
						break;
					case ID_KILL :
						this->_kill();
						break;
					case ID_EXIT:
						this->_exit();
						break;
				}
			}
		}
		break;

		// intercept the hittest message.. making full body of
		// window draggable.
		case WM_NCHITTEST:
		{
			// http://www.catch22.net/tuts/tips
			// this tests if you're on the non client area hit test
			UINT uHitTest = DefWindowProc(hwnd, WM_NCHITTEST, wParam, lParam);
			if(uHitTest == HTCLIENT)
				return HTCAPTION;
			else
				return uHitTest;
		}

		case WM_CLOSE:
			minimise() ;
			return 0;
			break;

		case WM_DESTROY:
			PostQuitMessage (0);
			break;

	}

	return DefWindowProc( hwnd, message, wParam, lParam ) ;
}

#endif	// REMOTE_LAUNCHER_GUI_HPP
