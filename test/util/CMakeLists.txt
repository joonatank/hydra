project( "util_test" CXX )

cmake_minimum_required(VERSION 2.8)

add_executable( test_args
	test_args.cpp
	${vl_SOURCE_DIR}/vl/base/args
	)

target_link_libraries( test_args  ${TEST_LIB} )

add_test( test_args ${PROJECT_BINARY_DIR}/test_args )
