#ifndef VL_FILESTRINGER_HPP
#define VL_FILESTRINGER_HPP

#include <iostream>
#include <fstream>

#include "rapidxml.hpp"

namespace vl
{
	
inline std::string readFileToString( std::string const &filePath )
{
    // Open in binary mode, so we don't mess up the file
    std::ifstream ifs( filePath.c_str(), std::ios::binary );

    return std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
}

inline bool writeFileFromString( std::string const &filePath, std::string const &content )
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

}

#endif // VL_FILESTRINGER_HPP
