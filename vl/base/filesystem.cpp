/**
 *	Copyright (c) 2010 - 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-11
 *	@file base/filesystem.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

// Header for this file
#include "filesystem.hpp"

// Necessary for readFile and writeFile
#include <fstream>
// Necessary for error printing
#include <iostream>

// Necessary for HYDRA_WIN32
#include "defines.hpp"
// Necessary for break_string_down
#include "string_utils.hpp"
// Necessary for log levels
#include "logger.hpp"

#ifdef _WIN32
#include <Windows.h>
#endif

std::string
vl::findPlugin( std::string const &plugin )
{
	// Ogre plugins have empty prefix
	std::string const prefix;
#ifdef HYDRA_WIN32
	std::string const postfix(".dll");
	char const delimiter(';');
	std::string paths = std::string(::getenv( "PATH" ));
	if(*(paths.end()-1) != delimiter)
	{ paths.append(std::string(1, delimiter)); }
	paths.append("./");
#else
	std::string const postfix(".so");
	char const delimiter(':');
	std::string paths = "/usr/lib:/usr/local/lib:./";
#endif
	std::string message = "Finding Plugins : paths = " + paths;
	std::cout << vl::TRACE << message << std::endl;

	std::vector<std::string> vec;
	vl::break_string_down( vec, paths, delimiter );
	std::vector<std::string>::iterator iter;
	std::string path;

	// Iterate through the path vector and find the plugin
	// TODO move the OGRE dir lookup somewhere else now this does the same thing
	// twice (once without the OGRE dir and once with it).
	for( iter = vec.begin(); iter != vec.end(); ++iter )
	{
		path = *iter + '/' + prefix + plugin + postfix;

		if( fs::exists( path ) )
		{ break; }

		path = *iter + '/' + "OGRE" + '/' + prefix + plugin + postfix;

		if( fs::exists( path ) )
		{ break; }
		path.clear();
	}

	if( !path.empty() )
	{
		// Plugin found
		std::cout << vl::TRACE << "Plugin found : " << path << std::endl;
	}

	return path;
}

bool
vl::find_file( const std::string &dir_path,
			   const std::string &filename,
			   std::string &path_found )
{
	if( !fs::exists( dir_path ) || !fs::is_directory( dir_path ) )
	{ return false; }

	fs::recursive_directory_iterator end_iter;
	for( fs::recursive_directory_iterator iter(dir_path); iter != end_iter; ++iter )
	{
		if( iter->path().filename() == filename )
		{
			path_found = iter->path().string();
			return true;
		}
	}

	return false;
}

std::string
vl::readFileToString(const std::string& filePath)
{
	std::string tmp;
	readFileToString( filePath, tmp );
	return tmp;
}

void
vl::readFileToString( const std::string& filePath, std::string& output )
{
	// Open in binary mode, so we don't mess up the file
	std::ifstream ifs( filePath.c_str(),  std::ios::in | std::ios::binary );

	output.insert( output.end(), (std::istreambuf_iterator<char>(ifs)),
					   std::istreambuf_iterator<char>() );
}


bool
vl::writeFileFromString(const std::string& filePath, const std::string& content)
{
	std::ofstream file( filePath.c_str() );

	if(!file)
	{
		std::cout << vl::CRITICAL << "Cannot open " << filePath << " for writing" << std::endl;
		return false;
		}

	file << content;

	file.close();
	if(file.fail())
	{
		std::cout << vl::CRITICAL << "Writing to " << filePath << " failed" << std::endl;
		return false;
	}
	return true;
}

fs::path
vl::get_global_path(GLOBAL_PATH type)
{
	fs::path path;
#ifdef _WIN32
	const char *data = 0;
	switch(type)
	{
		case GP_APP_DATA:
		{
			data = ::getenv("LOCALAPPDATA");
			if(!data)
			{
				data = ::getenv("APPDATA");
			}
			assert(data);
			path = fs::path(data);
		}
		break;

		case GP_STARTUP:
		{
			const char *app_data = ::getenv("APPDATA");
			assert(app_data);
			path = app_data;

			path /= fs::path("/Microsoft/Windows/Start Menu/Programs/Startup");
		}
		break;

		case GP_EXE:
		{
			char strExePath[1024];
			GetModuleFileName(NULL, strExePath, 1024);
			path = strExePath;
		}
		break;

		default :
			std::cerr << "Not supported path type" << std::endl;
	}
#else
#error "Linux not supported"
#endif
	return path;
}
