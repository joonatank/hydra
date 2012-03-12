/**
 *	Copyright (c) 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2012-03
 *	@file remote_launcher_app.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 */

#include "remote_launcher.hpp"

#include "base/sleep.hpp"

// Windows GUI, needs to be after asio including
#ifdef _WIN32
#include "remote_launcher_gui.hpp"
#endif

#include <iostream>

#include <cstdint>
#include <string>

/// Config directory Local AppData

/// Commands
// install
// kill

#ifdef _WIN32

LauncherWindow *g_launcher_gui = 0;
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR args, int iCmdShow )
{
	// @todo Check that there isn't multiple instances of this program
	// atm crashes with unhandled exception.

	Options opt = getOptions();
	std::cout << "Starting remote launcher" << std::endl
		<< opt << std::endl;

	opt.save_ini();

	if(!opt.valid())
	{
		std::cerr << "Can't start remote launcher because the config is invalid." << std::endl;
		return -1;
	}

	RemoteLauncher launcher(opt, vl::get_global_path(vl::GP_EXE).string());
	
	g_launcher_gui = new LauncherWindow(&launcher, hInstance, iCmdShow);

	MSG msg ;
	while (GetMessage (&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		launcher.mainloop();
		vl::msleep(1);
	}

	delete g_launcher_gui;

	return msg.wParam;
}
#else
int main(int argc, char **argv)
{
	if(args > 1)
	{
		std::string cmd(argv[1]);
		if( cmd == std::string("install") )
		{
			if(install() < 0)
			{
				std::cout << "Installation FAILED" << std::endl;
				return -1;
			}
			else
			{
				std::cout << "Installation successfull" << std::endl;
			}
		}
		else if( cmd == std::string("kill") )
		{
			std::cout << "Should kill" << std::endl;
		}
		else
		{
			std::cout << "Not supported command : " << cmd << std::endl;
		}
	}
	else
	{
		Options opt = getOptions();
		std::cout << "Starting remote launcher" << std::endl
			<< opt << std::endl;

		opt.save_ini();

		if(!opt.valid())
		{
			std::cerr << "Can't start remote launcher because the config is invalid." << std::endl;
			return -1;
		}

		RemoteLauncher launcher(opt);

		while (true)
		{
			launcher.mainloop();
			vl::msleep(1);
		}
	}
}
#endif

