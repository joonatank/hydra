
#include "arguments.hpp"

#include <iostream>

int main( int argc, char const **argv )
{
	vl::Arguments arg( argc, argv );
	std::cout << "environment path = " << arg.env_path << std::endl;
	std::cout << "project path = " << arg.proj_path << std::endl;
	std::cout << "case name = " << arg.case_name << std::endl;
}