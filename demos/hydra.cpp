/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-10
 *	@file hydra.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */


// Necessary for vl::exceptions
#include "base/exceptions.hpp"

#include "system_tools/message_box.hpp"

#include <string>
#include <vector>

// Necessary for checking 32 and 64-bit
#include "defines.hpp"

bool add_paths(std::vector<std::string> paths)
{
	std::string path("PATH=");
	char * original = ::getenv("PATH");
	if(original)
	{
		path += std::string(original);
	}
	for(size_t i = 0; i < paths.size(); ++i)
	{
		path += (";" + paths.at(i));
	}

	if(::putenv(path.c_str()) != 0)
	{ return false; }

	return true;
}

int main( const int argc, char** argv )
{
	/// Only windows uses dynamic load libraries for now
#ifdef HYDRA_WIN32
	// @todo these should be in a generated header file
	// so it's compile time configurable and can be configured automatically with CMake
	// @todo these should use a separate search directory (main dir)
	std::vector<std::string> extra_paths;
	extra_paths.push_back("bin");
#ifdef HYDRA_DEBUG
#ifdef HYDRA_ARCHITECTURE_64
	extra_paths.push_back("C:/jotu/software_development/hydra_dependencies_libraries/libs/bin");
	extra_paths.push_back("C:/jotu/software_development/hydra_dependencies_libraries/libs/bin/debug");
	extra_paths.push_back("C:/jotu/software_development/hydra_dependencies_libraries/libs/OGRE/bin/debug");
	extra_paths.push_back("C:/jotu/software_development/hydra_dependencies_libraries/libs/CEGUI/bin/debug");
	extra_paths.push_back("C:/jotu/software_development/hydra_dependencies_libraries/libs/expat/bin/debug");
	extra_paths.push_back("C:/jotu/software_development/hydra_dependencies_libraries/libs/OIS/bin/debug");
#else
	extra_paths.push_back("C:/jotu/software_development/hydra_dependencies_libraries/libs32/bin");
	extra_paths.push_back("C:/jotu/software_development/hydra_dependencies_libraries/libs32/bin/debug");
	extra_paths.push_back("C:/jotu/software_development/hydra_dependencies_libraries/libs32/OGRE/bin/debug");
	extra_paths.push_back("C:/jotu/software_development/hydra_dependencies_libraries/libs32/CEGUI/bin/debug");
	extra_paths.push_back("C:/jotu/software_development/hydra_dependencies_libraries/libs32/expat/bin/debug");
	extra_paths.push_back("C:/jotu/software_development/hydra_dependencies_libraries/libs32/OIS/bin/debug");
#endif

	LPCTSTR const LIB_NAME = "HydraMain_d.dll";
#else	// HYDRA_DEBUG
#ifdef HYDRA_ARCHITECTURE_64
	std::clog << "Adding 64 bit paths." << std::endl;
	extra_paths.push_back("C:/jotu/software_development/hydra_dependencies_libraries/libs/bin");
	extra_paths.push_back("C:/jotu/software_development/hydra_dependencies_libraries/libs/bin/release");
	extra_paths.push_back("C:/jotu/software_development/hydra_dependencies_libraries/libs/OGRE/bin/release");
	extra_paths.push_back("C:/jotu/software_development/hydra_dependencies_libraries/libs/CEGUI/bin/release");
	extra_paths.push_back("C:/jotu/software_development/hydra_dependencies_libraries/libs/expat/bin/release");
	extra_paths.push_back("C:/jotu/software_development/hydra_dependencies_libraries/libs/OIS/bin/release");
#else	// HYDRA_ARCHITECTURE_64
	std::clog << "Adding 32 bit paths." << std::endl;
	extra_paths.push_back("C:/jotu/software_development/hydra_dependencies_libraries/libs32/bin");
	extra_paths.push_back("C:/jotu/software_development/hydra_dependencies_libraries/libs32/bin/release");
	extra_paths.push_back("C:/jotu/software_development/hydra_dependencies_libraries/libs32/OGRE/bin/release");
	extra_paths.push_back("C:/jotu/software_development/hydra_dependencies_libraries/libs32/CEGUI/bin/release");
	extra_paths.push_back("C:/jotu/software_development/hydra_dependencies_libraries/libs32/expat/bin/release");
	extra_paths.push_back("C:/jotu/software_development/hydra_dependencies_libraries/libs32/OIS/bin/release");
#endif	// HYDRA_ARCHITECTURE_64

	LPCTSTR const LIB_NAME = "HydraMain.dll";
#endif	// HYDRA_DEBUG

	if(!add_paths(extra_paths))
	{
		std::cout << "Problem with setting the path." << std::endl;
		return -1;
	}

	HMODULE hydra_library = LoadLibrary(LIB_NAME);
	if(hydra_library == NULL)
	{
		std::cout << "Couldn't open library : " << LIB_NAME << std::endl;
		return -1;
	}

	vl::ExceptionMessage  msg;
	FARPROC run = GetProcAddress(hydra_library,"Hydra_Run");
	if(run == NULL)
	{
		std::cout << "Didn't find function pointer for Hydra_Run." << std::endl;
		return -1;
	}
	else
	{
	   // cast initializer to its proper type and use
		typedef vl::ExceptionMessage(*hydra_run_function_type)(const int argc, char** argv);
		hydra_run_function_type hydra_run = (hydra_run_function_type)run;

		msg = hydra_run(argc, argv);
	}
#endif	// HYDRA_WIN32

	/// Show the exception message
	if(!msg.empty())
	{
		vl::MessageDialog dialog(msg.title, msg.message);
	}

#ifdef HYDRA_WIN32
	FreeLibrary(hydra_library);
#endif

	return 0;
}
