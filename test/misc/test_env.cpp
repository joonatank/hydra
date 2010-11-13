/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
 *
 */

#include <cstdlib>
#include <iostream>

#include "base/string_utils.hpp"

int main( void )
{
	std::string path = ::getenv( "PATH" );
	std::cout << "PATH = " << path << std::endl;

	std::vector< std::string > vec;
	vl::break_string_down( vec, path, ':' );
	std::cout << "Path broken down : " << std::endl;
	for( std::vector<std::string>::iterator iter = vec.begin();
		iter != vec.end(); ++iter )
	{
		std::cout << *iter << std::endl;
	}
	std::cout << std::endl;

	return 0;
}