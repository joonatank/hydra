project( "util_test" CXX )

add_executable( test_args
	test_args.cpp
	${vl_SOURCE_DIR}/vl/base/args
	)

add_executable( test_fs test_fs.cpp )

target_link_libraries( test_args ${Boost_LIBRARIES} )
target_link_libraries( test_fs ${Boost_LIBRARIES} )

add_test( test_args ${PROJECT_BINARY_DIR}/test_args )
