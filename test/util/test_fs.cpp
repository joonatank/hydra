#ifdef VL_UNIX
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MODULE test_filesystem

#include <boost/test/unit_test.hpp>

#include <boost/filesystem.hpp>
#include <iostream>

#include "base/filesystem.hpp"

BOOST_AUTO_TEST_CASE( parent )
{
	fs::path current( fs::current_path() );
	std::cout << "Current path = " << current << std::endl
		<< "current root directory = " << current.root_directory() << std::endl
		<< "current parent = " << current.parent_path() << std::endl
		<< "current absolute = " << current.relative_path() << std::endl
		<< "current filename = " << current.filename() << std::endl
		<< "current file_string = " << current.file_string() << std::endl
		<< "current directory_string = " << current.directory_string() << std::endl;

	fs::path root("/");
	BOOST_CHECK_EQUAL( root, current.root_path() );

	// Test wether we can traverse backwards
	fs::path path( "../../" );
	std::cout << "path = ../../" << std::endl
		<< "path root directory = " << path.root_directory() << std::endl
		<< "path parent = " << path.parent_path() << std::endl
		<< "path relative = " << path.relative_path() << std::endl;
}

BOOST_AUTO_TEST_CASE( find_parent_dir )
{
	fs::path current( fs::current_path() );
	std::cout << "Current path = " << current << std::endl;
	fs::path path = vl::find_parent_dir( "data", current );
	std::cout << "found path = " << path << std::endl;
}
