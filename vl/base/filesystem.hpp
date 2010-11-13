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
// TODO test on Windows
std::string findPlugin( std::string const &plugin );

/// Read the file in filePath and return that file in string
std::string
readFileToString( std::string const &filePath );

/// Append file in filePath to the output string
void
readFileToString( std::string &output, std::string const &filePath );

bool
writeFileFromString( std::string const &filePath, std::string const &content );

}	// namespace vl

#endif
