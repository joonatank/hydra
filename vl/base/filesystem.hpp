/**
 *	Copyright (c) 2010 - 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-11
 *	@file base/filesystem.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

 /*
 *	filesystem utilities
 *	functions to find specific files
 *	Uses boost::filesystem
 */

#ifndef HYDRA_BASE_FILESYSTEM_HPP
#define HYDRA_BASE_FILESYSTEM_HPP

//#define BOOST_FILESYSTEM_NO_DEPRECATED
#include <boost/filesystem.hpp>

#include <string>

namespace fs = boost::filesystem;

namespace vl
{

enum PATH_TYPE
{
	PATH_ABS,
	PATH_REL
};


/**	@brief Find a plugin from system path or environment path
 *	@param plugin file name (without the extension .dll or .so)
 *
 *	@return string containing the plugin path if found, empty string if not found
 *
 *	Find paths
 *	Both : {Environment file dir}/plugins, current dir
 *	Windows : ${PATH}, ${PATH}/OGRE
 *	Linux : /usr/lib, /usr/local/lib, /usr/lib/OGRE, /usr/local/lib/OGRE
 */
std::string findPlugin( std::string const &plugin );

/**	@brief Find file recursively goes through the dir_path provided
 *	@param dir_path the directory where to start recursive search
 *	@param filename the name of a file to find
 *	@param path_found reference where the path is stored if found
 *	@return true if filename is found from dir_path
 *			false if dir_path does not exist, is not directory or filename not found
 *
 *	If returns true path_found is set to the path found
 */
bool
find_file( std::string const &dir_path, std::string const &filename, std::string &path_found );

/** @brief Read a file return that file in string.
 *	@param filePath is the path to a file to read
 *	@return if a valid file was read an string containing the whole file
 *			if no valid file exists an empty string
 *	@exception none
 */
std::string
readFileToString( std::string const &filePath );

/** @brief Append a file to the output file.
 *	@param filePath is the path to a file to read
 *	@param output is the string where the file should be appended to
 *	@exception none
 *
 *	If a valid file was read output has the content of that file appended to it
 *	if no valid file was found and read the output stays unchanged.
 */
void
readFileToString( std::string const &filePath, std::string &output );

/** @brief Write file content from string to a file in filesystem.
 *	@param filePath is the path to a file to write
 *	@param content is the data that should be writen to the file
 *	@return true if the filePath was valid and content was writen
 *			false otherwise
 *	@exception none
 *
 *	If file path contains a file already that file is overwriten.
 */
bool
writeFileFromString( std::string const &filePath, std::string const &content );

enum GLOBAL_PATH
{
	GP_APP_DATA,
	GP_TMP,
	GP_STARTUP,
	GP_EXE,
};

/// @brief Get the application data directory where config files are stored
/// @todo support for non ASCII characters missing
/// @brief Get the startup directory on Windows system
/// storing a symlink or an exe in this directory will make the system to execute it when started
/// @todo support for non ASCII characters missing
fs::path
get_global_path(GLOBAL_PATH type);

}	// namespace vl

#endif	// HYDRA_BASE_FILESYSTEM_HPP
