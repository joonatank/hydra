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

#include "daemon.hpp"

#include "res/resource.hpp"

// Windows headers
#include <windows.h>
#include <shellapi.h>

#define ID_TRAY_APP_ICON                5000
#define ID_TRAY_EXIT_CONTEXT_MENU_ITEM  3000
#define WM_TRAYICON ( WM_USER + 1 )

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

	HINSTANCE _instance;
	HWND _hwnd;
	HMENU _tray_menu;
	HMENU _main_menu;
	NOTIFYICONDATA _notifyIconData;

	// Taskbar created message
	UINT WM_TASKBARCREATED;
};

extern LauncherWindow *g_launcher_gui;



#endif	// REMOTE_LAUNCHER_GUI_HPP
