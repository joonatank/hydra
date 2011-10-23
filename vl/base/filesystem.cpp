/**
 *	Copyright (c) 2010-2011 Tampere University of Technology
 *	Copyright (c) 2011-10 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-11
 *	@file base/filesystem.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
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
// Necessary for creating file path with pid
#include "system_util.hpp"
// Necessary for log levels
#include "logger.hpp"

std::string
vl::createLogFilePath( const std::string &project_name,
					   const std::string &identifier,
					   const std::string &prefix,
					   const std::string &log_dir )
{
	uint32_t pid = vl::getPid();
	std::stringstream ss;

	if( !log_dir.empty() )
	{ ss << log_dir << "/"; }

	if( project_name.empty() )
	{ ss << "unamed"; }
	else
	{ ss << project_name; }

	if( !identifier.empty() )
	{ ss << '_' << identifier; }

	ss << '_' << pid;

	if( !prefix.empty() )
	{ ss << '_' << prefix; }

	ss << ".log";

	return ss.str();
}

std::string
vl::findPlugin( std::string const &plugin )
{
	// Ogre plugins have empty prefix
	std::string const prefix;
#ifdef HYDRA_WIN32
	std::string const postfix(".dll");
	char const delimiter(';');
	std::string paths = std::string(::getenv( "PATH" )) + ';' + std::string("./");
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
