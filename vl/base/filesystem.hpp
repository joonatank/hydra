/**	Joonatan Kuosa
 *	2010-11
 *
 *	filesystem utilities
 *	functions to find specific files
 *	Uses boost::filesystem
 **/

#ifndef VL_BASE_FILESYSTEM_HPP
#define VL_BASE_FILESYSTEM_HPP

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

std::string
createLogFilePath( const std::string &project_name,
					   const std::string &identifier,
					   const std::string &prefix = std::string(),
					   const std::string &log_dir = std::string() );

/**	Find a plugin from system path or environment path
 *	Input: plugin file name (without the extension .dll or .so)

 *	Return value : string containing the plugin path if found
 *	 			   empty string if not found
 *
 *	Find paths
 *	Both : {Environment file dir}/plugins, current dir
 *	Windows : ${PATH}, ${PATH}/OGRE
 *	Linux : /usr/lib, /usr/local/lib, /usr/lib/OGRE, /usr/local/lib/OGRE
 **/
std::string findPlugin( std::string const &plugin );

/**	Find file
 *	recursively goes through the dir_path provided
 *
 *	Returns true if filename is found from dir_path
 *	Returns false if dir_path does not exist, is not directory or filename not found
 *	If returns true path_found is set to the path found
 */
bool
find_file( std::string const &dir_path, std::string const &filename, std::string &path_found );

/// Read the file in filePath and return that file in string
std::string
readFileToString( std::string const &filePath );

/// Append file in filePath to the output string
void
readFileToString( std::string const &filePath, std::string &output );

bool
writeFileFromString( std::string const &filePath, std::string const &content );

}	// namespace vl

#endif
