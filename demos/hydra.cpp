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


#include "system_tools/message_box.hpp"

#include <string>
#include <vector>

// Hydra includes
// Necessary for vl::exceptions
#include "base/exceptions.hpp"
#include "base/filesystem.hpp"

// Necessary for checking 32 and 64-bit
#include "defines.hpp"

// Auto generated file that has information about the build environment
#include "build_defines.hpp"

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

	fs::path exe_dir = vl::get_global_path(vl::GP_EXE);
	exe_dir.remove_leaf();
	extra_paths.push_back(exe_dir.string());
	extra_paths.push_back(fs::path(exe_dir / "bin").string());

#ifdef HYDRA_DEBUG
	
	extra_paths.push_back(std::string(OGRE_BINARY_DIR) + "/debug");
	extra_paths.push_back(std::string(CEGUI_BINARY_DIR) + "/debug");
	extra_paths.push_back(std::string(OIS_BINARY_DIR) + "/debug");
	extra_paths.push_back(std::string(GENERAL_BINARY_DIR) + "/debug");
	extra_paths.push_back(std::string(BOOST_BINARY_DIR));
	extra_paths.push_back(std::string(EXPAT_BINARY_DIR) + "/debug");
	extra_paths.push_back(std::string(SKYX_BINARY_DIR) + "/debug");
	extra_paths.push_back(std::string(CAELUM_BINARY_DIR) + "/debug");
	extra_paths.push_back(std::string(HYDRA_MAIN_BINARY_DIR) + "/debug");
	extra_paths.push_back(std::string(HYDRA_GL_BINARY_DIR) + "/debug");

	LPCTSTR const LIB_NAME = "HydraMain_d.dll";
#else	// HYDRA_DEBUG
	
	extra_paths.push_back(std::string(OGRE_BINARY_DIR) + "/release");
	extra_paths.push_back(std::string(CEGUI_BINARY_DIR) + "/release");
	extra_paths.push_back(std::string(OIS_BINARY_DIR) + "/release");
	extra_paths.push_back(std::string(GENERAL_BINARY_DIR) + "/release");
	extra_paths.push_back(std::string(BOOST_BINARY_DIR));
	extra_paths.push_back(std::string(EXPAT_BINARY_DIR) + "/release");
	extra_paths.push_back(std::string(SKYX_BINARY_DIR) + "/release");
	extra_paths.push_back(std::string(CAELUM_BINARY_DIR) + "/release");
	extra_paths.push_back(std::string(HYDRA_MAIN_BINARY_DIR) + "/release");
	extra_paths.push_back(std::string(HYDRA_GL_BINARY_DIR) + "/release");

	LPCTSTR const LIB_NAME = "HydraMain.dll";
#endif	// HYDRA_DEBUG

	if(!add_paths(extra_paths))
	{
		std::string title("Hydra Error");
		std::string msg("Problem with setting the path.");
		vl::MessageDialog dialog(title, msg);
		return -1;
	}

	HMODULE hydra_library = LoadLibrary(LIB_NAME);
	if(hydra_library == NULL)
	{
		std::string title("Hydra Error");
		std::string msg("Couldn't open library : ");
		msg += LIB_NAME;
		vl::MessageDialog dialog(title, msg);
		return -1;
	}

	vl::ExceptionMessage  msg;
	FARPROC run = GetProcAddress(hydra_library,"Hydra_Run");
	if(run == NULL)
	{
		std::string title("Hydra Error");
		std::string msg("Didn't find function pointer for Hydra_Run.");
		vl::MessageDialog dialog(title, msg);
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
