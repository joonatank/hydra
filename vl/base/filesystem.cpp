
// Header for this file
#include "filesystem.hpp"

// Necessary for readFile and writeFile
#include <fstream>

// Necessary for error printing
#include <iostream>

// Necessary for break_string_down
#include "string_utils.hpp"


std::string
vl::findPlugin( std::string const &plugin )
{
	// Ogre plugins have empty prefix
	std::string const prefix;
#ifdef VL_WIN32
	std::string const postfix(".dll");
	char const delimiter(';');
	std::string paths = std::string(::getenv( "PATH" )) + ';' + std::string("./");
#else
	std::string const postfix(".so");
	char const delimiter(':');
	std::string paths = "/usr/lib:/usr/local/lib:./";
#endif
//	std::stringstream ss;
	std::cout << "Finding Plugins : paths = " << paths << std::endl;

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
		{
			// Plugin found
			std::cout << "Plugin found : " << path << std::endl;
			break;
		}

		path = *iter + '/' + "OGRE" + '/' + prefix + plugin + postfix;

		if( fs::exists( path ) )
		{
			break;
		}
		path.clear();
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
			path_found = iter->path().file_string();
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
		std::cerr << "Cannot open " << filePath << " for writing" << std::endl;
		return false;
		}

	file << content;

	file.close();
	if(file.fail())
	{
		std::cerr << "Writing to " << filePath << " failed" << std::endl;
		return false;
	}
	return true;
}
